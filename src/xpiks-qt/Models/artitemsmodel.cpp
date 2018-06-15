/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QCoreApplication>
#include <QStringList>
#include <QDirIterator>
#include <QImageReader>
#include <QSyntaxHighlighter>
#include <QList>
#include <QHash>
#include <QSet>
#include <vector>
#include <memory>
#include "artitemsmodel.h"
#include "artworkelement.h"
#include "../Helpers/indiceshelper.h"
#include "../Commands/addartworkscommand.h"
#include "../Commands/removeartworkscommand.h"
#include "../Commands/pastekeywordscommand.h"
#include "../Suggestion/keywordssuggestor.h"
#include "../Commands/commandmanager.h"
#include "artworksrepository.h"
#include "../Models/settingsmodel.h"
#include "../SpellCheck/spellcheckiteminfo.h"
#include "../Common/flags.h"
#include "../Commands/combinededitcommand.h"
#include "../Common/defines.h"
#include "../QMLExtensions/colorsmodel.h"
#include "imageartwork.h"
#include "../Commands/expandpresetcommand.h"
#include "../Helpers/constants.h"
#include "../Helpers/stringhelper.h"
#include "../QuickBuffer/quickbuffer.h"
#include "videoartwork.h"
#include "../QMLExtensions/artworkupdaterequest.h"
#include "../Helpers/filehelpers.h"
#include "../AutoComplete/keywordsautocompletemodel.h"
#include "../AutoComplete/completionitem.h"
#include "../Models/switchermodel.h"
#include "../Models/recentfilesmodel.h"
#include "../Helpers/artworkshelpers.h"
#include "../UndoRedo/addartworksitem.h"

namespace Models {
    ArtItemsModel::ArtItemsModel(ArtworksRepository &repository, QObject *parent):
        AbstractListModel(parent),
        Common::BaseEntity(),
        m_ArtworksRepository(repository),
        // all items before 1024 are reserved for internal models
        m_LastID(1024)
    { }

    ArtItemsModel::~ArtItemsModel() {
        for (auto *artwork: m_ArtworkList) {
            if (artwork->release()) {
                delete artwork;
            } else {
                m_FinalizationList.push_back(artwork);
            }
        }

#if defined(QT_DEBUG) && !defined(INTEGRATION_TESTS)
        // do not delete in release in order not to crash
        // if artwork locks were still kept by other entities
        qDeleteAll(m_FinalizationList);
#endif
    }

    ArtItemsModel::ArtworksAddResult ArtItemsModel::addFiles(const std::shared_ptr<Filesystem::IFilesCollection> &filesCollection,
                                                             Common::AddFilesFlags flags) {
        const int newFilesCount = m_ArtworksRepository.getNewFilesCount(filesCollection);
        MetadataIO::ArtworksSnapshot snapshot;
        snapshot.reserve(newFilesCount);
        qint64 directoryID = 0;
        const int count = getArtworksCount();

        beginAccountingFiles(newFilesCount);
        {
            for (auto &file: filesCollection->getFiles()) {
                if (m_ArtworksRepository.accountFile(file.m_Path, directoryID, directoryFlags)) {
                    ArtworkMetadata *artwork = nullptr;
                    if (file.m_Type == Filesystem::ArtworkFileType::Image) {
                        artwork = new ImageArtwork(file.m_Path, getNextID(), directoryID);
                    } else if (file.m_Type == Filesystem::ArtworkFileType::Video) {
                        artwork = new VideoArtwork(file, getNextID(), directoryID);
                    }

                    appendArtwork(artwork);
                    connectArtworkSignals(artwork);
                }
            }
        }
        endAccountingFiles();

        const bool autoAttach = Common::HasFlag(flags, Common::AddFilesFlags::FlagAutoFindVectors);
        int attachedCount = attachVectors(filesCollection, snapshot, count, autoAttach);
        m_ArtworksRepository.addFiles(snapshot);

        //syncArtworksIndices();

        return {
            snapshot,
                    attachedCount
        };
    }

    ArtItemsModel::ArtworksRemoveResult ArtItemsModel::removeFiles(const QVector<int> &indices) {
        QVector<QPair<int, int> > ranges;
        QVector<int> sortedIndices(indices);
        qSort(sortedIndices);
        Helpers::indicesToRanges(sortedIndices, ranges);
        auto snapshot = removeArtworks(ranges);
        auto removeResult = m_ArtworksRepository.removeFiles(snapshot);
        emit modifiedArtworksCountChanged();
        return {
            snapshot,
                    sortedIndices,
                    std::get<0>(removeResult), // directories ids set
                    std::get<1>(removeResult) // unselect all
        };
    }

    std::unique_ptr<MetadataIO::SessionSnapshot> ArtItemsModel::snapshotAll() {
        std::unique_ptr<MetadataIO::SessionSnapshot> sessionSnapshot(
                    new MetadataIO::SessionSnapshot(
                        m_ArtworkList,
                        m_ArtworksRepository.retrieveFullDirectories()));
        return sessionSnapshot;
    }

    int ArtItemsModel::getNextID() {
        return m_LastID++;
    }

    MetadataIO::ArtworksSnapshot ArtItemsModel::removeArtworks(const QVector<QPair<int, int> > &ranges) {
        MetadataIO::ArtworksSnapshot snapshot;
        int selectedCount = 0;

        for (auto &r: ranges) {
            Q_ASSERT(r.first >= 0 && r.first < getArtworksCount());
            Q_ASSERT(r.second >= 0 && r.second < getArtworksCount());
            Q_ASSERT(r.first <= r.second);

            auto itBegin = m_ArtworkList.begin() + r.first;
            auto itEnd = m_ArtworkList.begin() + (r.second + 1);

            std::vector<ArtworkMetadata *> itemsToDelete(itBegin, itEnd);
            m_ArtworkList.erase(itBegin, itEnd);

            std::vector<ArtworkMetadata *>::iterator it = itemsToDelete.begin();
            std::vector<ArtworkMetadata *>::iterator itemsEnd = itemsToDelete.end();
            for (; it < itemsEnd; it++) {
                ArtworkMetadata *artwork = *it;
                snapshot.append(artwork);
                m_ArtworksRepository.removeFile(artwork->getFilepath(), artwork->getDirectoryID());
                if (artwork->isSelected()) {
                    selectedCount++;
                }

                LOG_INTEGRATION_TESTS << "File removed:" << artwork->getFilepath();
                destroyInnerItem(artwork);
            }
        }

        if (selectedCount > 0) {
            emit selectedArtworksRemoved(selectedCount);
        }

        return snapshot;
    }

    int ArtItemsModel::attachVectors(const std::shared_ptr<Filesystem::IFilesCollection> &filesCollection,
                                     const MetadataIO::ArtworksSnapshot &snapshot,
                                     int initialCount,
                                     bool autoAttach) {
        QHash<QString, QHash<QString, QString> > vectors;
        for (auto &path: filesCollection->getVectors()) {
            QFileInfo fi(path);
            const QString &absolutePath = fi.absolutePath();

            if (!vectors.contains(absolutePath)) {
                vectors.insert(absolutePath, QHash<QString, QString>());
            }

            vectors[absolutePath].insert(fi.baseName().toLower(), path);
        }

        QVector<int> indicesToUpdate;
        int attachedCount = this->attachVectors(vectors, indicesToUpdate);

        if (autoAttach) {
            QVector<int> autoAttachedIndices;
            attachedCount += Helpers::findAndAttachVectors(snapshot.getWeakSnapshot(), autoAttachedIndices);

            foreach (int index, autoAttachedIndices) {
                indicesToUpdate.append(initialCount + index);
            }
        }

        this->updateItems(indicesToUpdate, QVector<int>() << Models::ArtItemsModel::HasVectorAttachedRole);
        return attachedCount;
    }

    void ArtItemsModel::connectArtworkSignals(ArtworkMetadata *artwork) {
        QObject::connect(artwork, &ArtworkMetadata::modifiedChanged,
                         this, &ArtItemsModel::itemModifiedChanged);

        QObject::connect(artwork, &ArtworkMetadata::backupRequired,
                         this, &Models::ArtItemsModel::onArtworkBackupRequested);

        QObject::connect(artwork, &ArtworkMetadata::editingPaused,
                         this, &Models::ArtItemsModel::onArtworkEditingPaused);

        QObject::connect(artwork, &ArtworkMetadata::spellingInfoUpdated,
                         this, &ArtItemsModel::onArtworkSpellingInfoUpdated);

        QObject::connect(artwork, &ArtworkMetadata::selectedChanged,
                         this, &ArtItemsModel::artworkSelectedChanged);
    }

    void ArtItemsModel::deleteAllItems() {
        LOG_DEBUG << "#";
        // should be called only from beforeDestruction() !
        // will not cause sync issues on shutdown if no items
        decltype(m_ArtworkList) artworksToDestroy;

        beginResetModel();
        {
            artworksToDestroy.swap(m_ArtworkList);
            m_ArtworkList.clear();
        }
        endResetModel();

        size_t size = artworksToDestroy.size();
        for (size_t i = 0; i < size; ++i) {
            ArtworkMetadata *metadata = artworksToDestroy.at(i);
            destroyInnerItem(metadata);
        }
    }

#ifdef INTEGRATION_TESTS
    void ArtItemsModel::fakeDeleteAllItems() {
        LOG_DEBUG << "#";

        std::deque<ArtworkMetadata *> artworksToDelete;

        beginResetModel();
        {
            artworksToDelete.swap(m_ArtworkList);
            m_ArtworkList.clear();
        }
        endResetModel();

#ifdef QT_DEBUG
        for (auto *item: artworksToDelete) {
            item->deepDisconnect();
            m_DestroyedList.push_back(item);
        }
#endif
    }
#endif

    int ArtItemsModel::getModifiedArtworksCount() {
        int modifiedCount = 0;
        size_t size = m_ArtworkList.size();

        for (size_t i = 0; i < size; ++i) {
            if (accessArtwork(i)->isModified()) {
                modifiedCount++;
            }
        }

        return modifiedCount;
    }

    void ArtItemsModel::updateItems(const QVector<int> &indices, const QVector<int> &roles) {
        QVector<QPair<int, int> > rangesToUpdate;
        QVector<int> sortedIndices(indices);
        qSort(sortedIndices);
        Helpers::indicesToRanges(sortedIndices, rangesToUpdate);
        AbstractListModel::updateItemsInRanges(rangesToUpdate, roles);
    }

    void ArtItemsModel::forceUnselectAllItems() const {
        size_t count = m_ArtworkList.size();

        for (size_t i = 0; i < count; ++i) {
            accessArtwork(i)->resetSelected();
        }
    }

    bool ArtItemsModel::removeUnavailableItems() {
        LOG_DEBUG << "#";
        QVector<int> indicesToRemove;

        const size_t size = m_ArtworkList.size();
        for (size_t i = 0; i < size; ++i) {
            if (accessArtwork(i)->isUnavailable()) {
                indicesToRemove.append((int)i);
                emit fileWithIndexUnavailable(i);
            }
        }

        this->removeFiles(indicesToRemove);
        return !indicesToRemove.empty();
    }

    void ArtItemsModel::generateAboutToBeRemoved() {
        LOG_DEBUG << "#";
        size_t size = m_ArtworkList.size();

        for (size_t i = 0; i < size; ++i) {
            ArtworkMetadata *metadata = accessArtwork(i);

            if (metadata->isUnavailable()) {
                Common::BasicKeywordsModel *keywordsModel = metadata->getBasicModel();
                keywordsModel->notifyAboutToBeRemoved();
            }
        }
    }

    void ArtItemsModel::updateAllItems() {
        updateItemsInRanges(QVector<QPair<int, int> >() << qMakePair(0, (int)getArtworksCount() - 1));
    }

    void ArtItemsModel::removeArtworksDirectory(int index) {
        LOG_INFO << "Remove artworks directory at" << index;
        const QString &directory = m_ArtworksRepository.getDirectoryPath(index);
        const bool isFullDirectory = m_ArtworksRepository.getIsFullDirectory(index);
        LOG_CORE_TESTS << "Removing directory:" << directory << "; full:" << isFullDirectory;

        const QString directoryAbsolutePath = QDir(directory).absolutePath();
        QVector<int> indicesToRemove;
        const size_t size = m_ArtworkList.size();
        indicesToRemove.reserve((int)size);

        for (size_t i = 0; i < size; ++i) {
            ArtworkMetadata *metadata = accessArtwork(i);
            if (metadata->isInDirectory(directoryAbsolutePath)) {
                indicesToRemove.append((int)i);
            }
        }

        return removeFiles(indicesToRemove);
    }

    void ArtItemsModel::removeKeywordAt(int metadataIndex, int keywordIndex) {
        LOG_INFO << "metadata index" << metadataIndex << "| keyword index" << keywordIndex;
        if (0 <= metadataIndex && metadataIndex < getArtworksCount()) {
            ArtworkMetadata *metadata = accessArtwork(metadataIndex);
            QString removed;

            if (metadata->removeKeywordAt(keywordIndex, removed)) {
                QModelIndex index = this->index(metadataIndex);
                emit dataChanged(index, index, QVector<int>() << IsModifiedRole << KeywordsCountRole);
            }
        }
    }

    void ArtItemsModel::removeLastKeyword(int metadataIndex) {
        LOG_INFO << "index" << metadataIndex;
        if (0 <= metadataIndex && metadataIndex < getArtworksCount()) {
            ArtworkMetadata *metadata = accessArtwork(metadataIndex);
            QString removed;

            if (metadata->removeLastKeyword(removed)) {
                QModelIndex index = this->index(metadataIndex);
                emit dataChanged(index, index, QVector<int>() << IsModifiedRole << KeywordsCountRole);
            }
        }
    }

    bool ArtItemsModel::appendKeyword(int metadataIndex, const QString &keyword) {
        bool added = false;
        LOG_INFO << "metadata index" << metadataIndex << "| keyword" << keyword;

        if (0 <= metadataIndex && metadataIndex < getArtworksCount()) {
            ArtworkMetadata *metadata = accessArtwork(metadataIndex);

            if (metadata->appendKeyword(keyword)) {
                QModelIndex index = this->index(metadataIndex);
                emit dataChanged(index, index, QVector<int>() << IsModifiedRole << KeywordsCountRole);
                auto *keywordsModel = metadata->getBasicModel();

                xpiks()->submitKeywordForSpellCheck(keywordsModel, keywordsModel->getKeywordsCount() - 1);

                added = true;
            }
        }

        return added;
    }

    void ArtItemsModel::pasteKeywords(int metadataIndex, const QStringList &keywords) {
        LOG_INFO << "item index" << metadataIndex << "|" << keywords;
        if (metadataIndex >= 0
            && metadataIndex < getArtworksCount()
            && !keywords.empty()) {
            MetadataIO::ArtworksSnapshot::Container rawArtworkSnapshot;
            QVector<int> selectedIndices;

            // TODO: to be changed in future to the dialog
            // getSelectedItemsIndices(selectedIndices);
            // if (!metadata->getIsSelected()) {
            selectedIndices.append(metadataIndex);
            // }
            rawArtworkSnapshot.reserve(selectedIndices.size());

            bool onlyOneKeyword = keywords.length() == 1;

            if (onlyOneKeyword) {
                LOG_INFO << "Pasting only one keyword. Leaving it in the edit box.";
                return;
            }

            foreach(int index, selectedIndices) {
                ArtworkMetadata *metadata = accessArtwork(index);
                rawArtworkSnapshot.emplace_back(new ArtworkMetadataLocker(metadata));
            }

            std::shared_ptr<Commands::PasteKeywordsCommand> pasteCommand(new Commands::PasteKeywordsCommand(rawArtworkSnapshot, keywords));
            m_CommandManager->processCommand(pasteCommand);
        }
    }

    void ArtItemsModel::addSuggestedKeywords(int metadataIndex, const QStringList &keywords) {
        LOG_DEBUG << "item index" << metadataIndex;
        if (metadataIndex >= 0
            && metadataIndex < getArtworksCount()
            && !keywords.empty()) {
            MetadataIO::ArtworksSnapshot::Container rawSnapshot;

            ArtworkMetadata *metadata = accessArtwork(metadataIndex);
            rawSnapshot.emplace_back(new ArtworkMetadataLocker(metadata));

            std::shared_ptr<Commands::PasteKeywordsCommand> pasteCommand(new Commands::PasteKeywordsCommand(rawSnapshot, keywords));
            m_CommandManager->processCommand(pasteCommand);
        }
    }

    void ArtItemsModel::suggestCorrections(int metadataIndex) {
        if (0 <= metadataIndex && metadataIndex < getArtworksCount()) {
            using namespace Common;
            Common::SuggestionFlags flags = Common::SuggestionFlags::None;
            Common::SetFlag(flags, SuggestionFlags::Description);
            Common::SetFlag(flags, SuggestionFlags::Title);
            Common::SetFlag(flags, SuggestionFlags::Keywords);
            ArtworkMetadata *metadata = accessArtwork(metadataIndex);
            xpiks()->setupSpellCheckSuggestions(metadata, metadataIndex, flags);
        }
    }

    void ArtItemsModel::backupItem(int metadataIndex) {
        if (0 <= metadataIndex && metadataIndex < getArtworksCount()) {
            ArtworkMetadata *metadata = accessArtwork(metadataIndex);
            xpiks()->saveArtworkBackup(metadata);
        }
    }

    void ArtItemsModel::setSelectedItemsSaved(const QVector<int> &selectedIndices) {
        LOG_INFO << "Setting selected" << selectedIndices.length() << "item(s) saved";
        foreach(int index, selectedIndices) {
            accessArtwork(index)->resetModified();
        }

        QVector<QPair<int, int> > rangesToUpdate;
        Helpers::indicesToRanges(selectedIndices, rangesToUpdate);
        AbstractListModel::updateItemsInRanges(rangesToUpdate, QVector<int>() << IsModifiedRole);

        emit modifiedArtworksCountChanged();
        emit artworksChanged(false);
    }

    void ArtItemsModel::updateSelectedArtworks(const QVector<int> &selectedIndices) {
        QVector<int> roles;
        fillStandardRoles(roles);
        updateSelectedArtworksEx(selectedIndices, roles);
    }

    void ArtItemsModel::updateSelectedArtworksEx(const QVector<int> &selectedIndices, const QVector<int> roles) {
        QVector<QPair<int, int> > rangesToUpdate;
        Helpers::indicesToRanges(selectedIndices, rangesToUpdate);

        AbstractListModel::updateItemsInRanges(rangesToUpdate, roles);

        emit artworksChanged(false);
    }

    ArtworkMetadata *ArtItemsModel::getArtworkMetadata(int index) const {
        ArtworkMetadata *item = NULL;

        if (0 <= index && index < getArtworksCount()) {
            item = accessArtwork(index);
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        }

        return item;
    }

    Common::BasicMetadataModel *ArtItemsModel::getBasicModel(int index) const {
        Common::BasicMetadataModel *keywordsModel = NULL;

        if (0 <= index && index < getArtworksCount()) {
            keywordsModel = accessArtwork(index)->getBasicModel();
            QQmlEngine::setObjectOwnership(keywordsModel, QQmlEngine::CppOwnership);
        }

        return keywordsModel;
    }

    QSize ArtItemsModel::retrieveImageSize(int metadataIndex) const {
        if (metadataIndex < 0 || metadataIndex >= getArtworksCount()) {
            return QSize();
        }

        ArtworkMetadata *metadata = accessArtwork(metadataIndex);
        ImageArtwork *image = dynamic_cast<ImageArtwork *>(metadata);

        if (image == NULL) {
            return QSize();
        }

        QSize size;

        if (image->isInitialized()) {
            size = image->getImageSize();
        } else {
            QImageReader reader(image->getFilepath());
            size = reader.size();
            // metadata->setSize(size);
        }

        return size;
    }

    QString ArtItemsModel::retrieveFileSize(int metadataIndex) const {
        if (metadataIndex < 0 || metadataIndex >= getArtworksCount()) {
            return QLatin1String("-");
        }

        ArtworkMetadata *metadata = accessArtwork(metadataIndex);

        qint64 size = 0;

        if (metadata->isInitialized()) {
            size = metadata->getFileSize();
        } else {
            QFileInfo fi(metadata->getFilepath());
            size = fi.size(); // in bytes
        }

        QString sizeDescription = Helpers::describeFileSize(size);
        return sizeDescription;
    }

    QString ArtItemsModel::getArtworkFilepath(int metadataIndex) const {
        if (metadataIndex < 0 || metadataIndex >= getArtworksCount()) {
            return QLatin1String("");
        }

        ArtworkMetadata *metadata = accessArtwork(metadataIndex);
        return metadata->getFilepath();
    }

    QString ArtItemsModel::getAttachedVectorPath(int metadataIndex) const {
        if (metadataIndex < 0 || metadataIndex >= getArtworksCount()) {
            return QLatin1String("");
        }

        ArtworkMetadata *metadata = accessArtwork(metadataIndex);
        ImageArtwork *image = dynamic_cast<ImageArtwork *>(metadata);
        if (image != NULL) {
            return image->getAttachedVectorPath();
        } else {
            return QLatin1String("");
        }
    }

    QString ArtItemsModel::getArtworkDateTaken(int metadataIndex) const {
        if (metadataIndex < 0 || metadataIndex >= getArtworksCount()) {
            return QLatin1String("");
        }

        ArtworkMetadata *metadata = accessArtwork(metadataIndex);
        ImageArtwork *image = dynamic_cast<ImageArtwork *>(metadata);
        if (image != NULL) {
            return image->getDateTaken();
        } else {
            return QLatin1String("");
        }
    }

    int ArtItemsModel::addRecentDirectory(const QString &directory) {
        LOG_INFO << directory;
        int filesAdded = doAddDirectories(QStringList() << directory);
        return filesAdded;
    }

    int ArtItemsModel::addRecentFile(const QString &file) {
        LOG_INFO << file;
        int filesAdded = doAddFiles(QStringList() << file);
        return filesAdded;
    }

    int ArtItemsModel::addAllRecentFiles() {
        LOG_DEBUG << "#";
        Models::RecentFilesModel *recentFiles = m_CommandManager->getRecentFiles();
        int filesAdded = doAddFiles(recentFiles->getAllRecentFiles());
        return filesAdded;
    }

    void ArtItemsModel::initDescriptionHighlighting(int metadataIndex, QQuickTextDocument *document) {
        if (0 <= metadataIndex && metadataIndex < getArtworksCount()) {
            ArtworkMetadata *metadata = accessArtwork(metadataIndex);
            auto *metadataModel = metadata->getBasicModel();
            SpellCheck::SpellCheckItemInfo *info = metadataModel->getSpellCheckInfo();
            QMLExtensions::ColorsModel *colorsModel = m_CommandManager->getColorsModel();
            info->createHighlighterForDescription(document->textDocument(), colorsModel, metadataModel);
            metadataModel->notifyDescriptionSpellingChanged();
        }
    }

    void ArtItemsModel::initTitleHighlighting(int metadataIndex, QQuickTextDocument *document) {
        if (0 <= metadataIndex && metadataIndex < getArtworksCount()) {
            ArtworkMetadata *metadata = accessArtwork(metadataIndex);
            auto *metadataModel = metadata->getBasicModel();
            SpellCheck::SpellCheckItemInfo *info = metadataModel->getSpellCheckInfo();
            QMLExtensions::ColorsModel *colorsModel = m_CommandManager->getColorsModel();
            info->createHighlighterForTitle(document->textDocument(), colorsModel, metadataModel);
            metadataModel->notifyTitleSpellingChanged();
        }
    }

    void ArtItemsModel::editKeyword(int metadataIndex, int keywordIndex, const QString &replacement) {
        LOG_INFO << "metadata index:" << metadataIndex;
        if (0 <= metadataIndex && metadataIndex < getArtworksCount()) {
            ArtworkMetadata *metadata = accessArtwork(metadataIndex);
            if (metadata->editKeyword(keywordIndex, replacement)) {
                QModelIndex index = this->index(metadataIndex);
                emit dataChanged(index, index, QVector<int>() << IsModifiedRole << KeywordsCountRole);

                auto *keywordsModel = metadata->getBasicModel();
                xpiks()->submitKeywordForSpellCheck(keywordsModel, keywordIndex);
            }
        }
    }

    void ArtItemsModel::plainTextEdit(int metadataIndex, const QString &rawKeywords, bool spaceIsSeparator) {
        LOG_DEBUG << "Plain text edit for item" << metadataIndex;
        if (0 <= metadataIndex && metadataIndex < getArtworksCount()) {
            ArtworkMetadata *metadata = accessArtwork(metadataIndex);

            QVector<QChar> separators;
            separators << QChar(',');
            if (spaceIsSeparator) { separators << QChar::Space; }
            QStringList keywords;
            Helpers::splitKeywords(rawKeywords.trimmed(), separators, keywords);

            MetadataIO::ArtworksSnapshot::Container items;
            items.emplace_back(new ArtworkMetadataLocker(metadata));

            Common::CombinedEditFlags flags = Common::CombinedEditFlags::None;
            Common::SetFlag(flags, Common::CombinedEditFlags::EditKeywords);
            std::shared_ptr<Commands::CombinedEditCommand> combinedEditCommand(new Commands::CombinedEditCommand(
                    flags,
                    items,
                    "", "",
                    keywords));

            m_CommandManager->processCommand(combinedEditCommand);
            updateItemAtIndex(metadataIndex);
        }
    }

    void ArtItemsModel::detachVectorsFromArtworks(const QVector<int> &indices) {
        LOG_INFO << indices.size() << "indices";
        QVector<int> indicesToUpdate;
        indicesToUpdate.reserve(indices.length());
        Models::ArtworksRepository *artworksRepository = m_CommandManager->getArtworksRepository();

        for(int index: indices) {
            ArtworkMetadata *metadata = accessArtwork(index);
            ImageArtwork *image = dynamic_cast<ImageArtwork *>(metadata);

            if (image != NULL) {
                const QString vectorPath = image->getAttachedVectorPath();
                image->detachVector();
                artworksRepository->removeVector(vectorPath);
                indicesToUpdate.append(index);
            }
        }

        LOG_INFO << indicesToUpdate.length() << "item(s) affected";

        if (!indicesToUpdate.isEmpty()) {
            QVector<QPair<int, int> > rangesToUpdate;
            Helpers::indicesToRanges(indicesToUpdate, rangesToUpdate);
            AbstractListModel::updateItemsInRanges(rangesToUpdate, QVector<int>() << HasVectorAttachedRole);
        }
    }

    void ArtItemsModel::expandPreset(int artworkIndex, int keywordIndex, unsigned int presetID) {
        LOG_INFO << "item" << artworkIndex << "keyword" << keywordIndex << "preset" << presetID;

        if (0 <= artworkIndex && artworkIndex < getArtworksCount()) {
            ArtworkMetadata *artwork = accessArtwork(artworkIndex);
            std::shared_ptr<Commands::ExpandPresetCommand> expandPresetCommand(new Commands::ExpandPresetCommand(artwork, (KeywordsPresets::ID_t)presetID, keywordIndex));
            std::shared_ptr<Commands::ICommandResult> result = m_CommandManager->processCommand(expandPresetCommand);
            Q_UNUSED(result);
        }
    }

    void ArtItemsModel::expandLastAsPreset(int metadataIndex) {
        LOG_INFO << "item" << metadataIndex;

        if (0 <= metadataIndex && metadataIndex < getArtworksCount()) {
            ArtworkMetadata *artwork = accessArtwork(metadataIndex);
            auto *basicModel = artwork->getBasicModel();
            int keywordIndex = basicModel->getKeywordsCount() - 1;
            QString lastKeyword = basicModel->retrieveKeyword(keywordIndex);

            auto *presetsModel = m_CommandManager->getPresetsModel();
            KeywordsPresets::ID_t presetID;
            if (presetsModel->tryFindSinglePresetByName(lastKeyword, false, presetID)) {
                std::shared_ptr<Commands::ExpandPresetCommand> expandPresetCommand(new Commands::ExpandPresetCommand(artwork, presetID, keywordIndex));
                std::shared_ptr<Commands::ICommandResult> result = m_CommandManager->processCommand(expandPresetCommand);
                Q_UNUSED(result);
            }
        }
    }

    void ArtItemsModel::addPreset(int metadataIndex, unsigned int presetID) {
        LOG_INFO << "item" << metadataIndex << "preset" << presetID;

        if (0 <= metadataIndex && metadataIndex < rowCount()) {
            ArtworkMetadata *artwork = accessArtwork(metadataIndex);
            std::shared_ptr<Commands::ExpandPresetCommand> expandPresetCommand(new Commands::ExpandPresetCommand(artwork, presetID));
            std::shared_ptr<Commands::ICommandResult> result = m_CommandManager->processCommand(expandPresetCommand);
            Q_UNUSED(result);
        }
    }

    bool ArtItemsModel::acceptCompletionAsPreset(int metadataIndex, int completionID) {
        LOG_INFO << "item" << metadataIndex << "completionID" << completionID;
        bool accepted = false;

        if (0 <= metadataIndex && metadataIndex < rowCount()) {
            ArtworkMetadata *artwork = accessArtwork(metadataIndex);

            AutoComplete::KeywordsAutoCompleteModel *acModel = m_CommandManager->getAutoCompleteModel();
            std::shared_ptr<AutoComplete::CompletionItem> completionItem = acModel->getAcceptedCompletion(completionID);
            if (!completionItem) {
                LOG_WARNING << "Completion is not available anymore";
                return false;
            }

            const int presetID = completionItem->getPresetID();

            if (completionItem->isPreset() ||
                    (completionItem->canBePreset() && completionItem->shouldExpandPreset())) {
                std::shared_ptr<Commands::ExpandPresetCommand> expandPresetCommand(new Commands::ExpandPresetCommand(artwork, presetID));
                std::shared_ptr<Commands::ICommandResult> result = m_CommandManager->processCommand(expandPresetCommand);
                Q_UNUSED(result);
                accepted = true;
            }/* --------- this is handled in the edit field -----------
                else if (completionItem->isKeyword()) {
                this->appendKeyword(metadataIndex, completionItem->getCompletion());
                accepted = true;
            }*/
        }

        return accepted;
    }

    void ArtItemsModel::initSuggestion(int metadataIndex) {
        LOG_INFO << "item" << metadataIndex;
#ifndef CORE_TESTS
        if (0 <= metadataIndex && metadataIndex < rowCount()) {
            auto *suggestor = m_CommandManager->getKeywordsSuggestor();
            ArtworkMetadata *metadata = accessArtwork(metadataIndex);
            suggestor->setExistingKeywords(metadata->getKeywordsSet());
        }
#endif
    }

    void ArtItemsModel::setupDuplicatesModel(int metadataIndex) {
        LOG_INFO << metadataIndex;
        if (0 <= metadataIndex && metadataIndex < rowCount()) {
            ArtworkMetadata *artwork = accessArtwork(metadataIndex);
            Q_ASSERT(artwork != nullptr);
            std::vector<ArtworkMetadata*> artworks;
            artworks.push_back(artwork);
            xpiks()->setupDuplicatesModel(artworks);
        }
    }

    bool ArtItemsModel::hasModifiedArtworks() const {
        LOG_DEBUG << "#";

        bool anyModified = false;
        const size_t size = m_ArtworkList.size();

        for (size_t i = 0; i < size; ++i) {
            if (accessArtwork(i)->isModified()) {
                anyModified = true;
                break;
            }
        }

        return anyModified;
    }

    void ArtItemsModel::fillFromQuickBuffer(size_t metadataIndex) {
        LOG_INFO << "item" << metadataIndex;

        if (metadataIndex < m_ArtworkList.size()) {
            ArtworkMetadata *artwork = accessArtwork(metadataIndex);
            auto *quickBuffer = m_CommandManager->getQuickBuffer();

            MetadataIO::ArtworksSnapshot::Container items;
            items.emplace_back(new ArtworkMetadataLocker(artwork));

            Common::CombinedEditFlags flags = Common::CombinedEditFlags::None;

            QString title = quickBuffer->getTitle();
            QString description = quickBuffer->getDescription();
            QStringList keywords = quickBuffer->getKeywords();

            if (!title.isEmpty()) { Common::SetFlag(flags, Common::CombinedEditFlags::EditTitle); }
            if (!description.isEmpty()) { Common::SetFlag(flags, Common::CombinedEditFlags::EditDescription); }
            if (!keywords.empty()) { Common::SetFlag(flags, Common::CombinedEditFlags::EditKeywords); }

            std::shared_ptr<Commands::CombinedEditCommand> combinedEditCommand(new Commands::CombinedEditCommand(
                                                                                   flags, items, description, title, keywords));

            m_CommandManager->processCommand(combinedEditCommand);
            updateItemAtIndex((int)metadataIndex);
        }
    }

    int ArtItemsModel::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return (int)getArtworksCount();
    }

    QVariant ArtItemsModel::data(const QModelIndex &index, int role) const {
        int row = index.row();

        if (row < 0 || row >= getArtworksCount()) {
            return QVariant();
        }

        ArtworkMetadata *artwork = accessArtwork(row);
        switch (role) {
            case ArtworkDescriptionRole:
                return artwork->getDescription();
            case ArtworkFilenameRole:
                return artwork->getFilepath();
            case ArtworkTitleRole:
                return artwork->getTitle();
            case KeywordsStringRole: {
                Common::BasicKeywordsModel *keywordsModel = artwork->getBasicModel();
                return keywordsModel->getKeywordsString();
            }
            case IsModifiedRole:
                return artwork->isModified();
            case IsSelectedRole:
                return artwork->isSelected();
            case KeywordsCountRole: {
                Common::BasicKeywordsModel *keywordsModel = artwork->getBasicModel();
                return keywordsModel->getKeywordsCount();
            }
            case HasVectorAttachedRole: {
                ImageArtwork *image = dynamic_cast<ImageArtwork *>(artwork);
                return (image != NULL) && image->hasVectorAttached();
            }
            case BaseFilenameRole:
                return artwork->getBaseFilename();
            case IsVideoRole: {
                bool isVideo = dynamic_cast<VideoArtwork*>(artwork) != nullptr;
                return isVideo;
            }
            case ArtworkThumbnailRole:
                return artwork->getThumbnailPath();
            case IsReadOnlyRole:
                return artwork->isReadOnly();
            default:
                return QVariant();
        }
    }

    Qt::ItemFlags ArtItemsModel::flags(const QModelIndex &index) const {
        int row = index.row();

        if (row < 0 || row >= getArtworksCount()) {
            return Qt::ItemIsEnabled;
        }

        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    bool ArtItemsModel::setData(const QModelIndex &index, const QVariant &value, int role) {
        int row = index.row();

        if (row < 0 || row >= getArtworksCount()) {
            return false;
        }

        ArtworkMetadata *metadata = accessArtwork(row);
        if (metadata->isLockedForEditing()) { return false; }

        int roleToUpdate = 0;
        bool needToUpdate = false;
        switch (role) {
            case EditArtworkDescriptionRole:
            LOG_FOR_DEBUG << "item" << row << "description" << value;
                needToUpdate = metadata->setDescription(value.toString());
                roleToUpdate = ArtworkDescriptionRole;
                break;
            case EditArtworkTitleRole:
            LOG_FOR_DEBUG << "item" << row << "title" << value;
                needToUpdate = metadata->setTitle(value.toString());
                roleToUpdate = ArtworkTitleRole;
                break;
            case EditIsSelectedRole:
            LOG_FOR_DEBUG << "item" << row << "isSelected" << value;
                needToUpdate = metadata->setIsSelected(value.toBool());
                roleToUpdate = IsSelectedRole;
                break;
            default:
                return false;
        }

        if (needToUpdate) {
            emit dataChanged(index, index, QVector<int>() << IsModifiedRole << roleToUpdate);
        }

        return true;
    }

    void ArtItemsModel::onFilesUnavailableHandler() {
        LOG_DEBUG << "#";
        Models::ArtworksRepository *artworksRepository = m_CommandManager->getArtworksRepository();
        size_t count = getArtworksCount();

        bool anyArtworkUnavailable = false;
        bool anyVectorUnavailable = false;

        for (size_t i = 0; i < count; ++i) {
            ArtworkMetadata *artwork = accessArtwork(i);
            ImageArtwork *image = dynamic_cast<ImageArtwork *>(artwork);
            const QString &path = artwork->getFilepath();

            if (artworksRepository->isFileUnavailable(path)) {
                artwork->setUnavailable();
                anyArtworkUnavailable = true;
            } else if (image != NULL && image->hasVectorAttached()) {
                const QString &vectorPath = image->getAttachedVectorPath();
                if (artworksRepository->isFileUnavailable(vectorPath)) {
                    image->detachVector();
                    anyVectorUnavailable = true;
                }
            }
        }

        if (anyArtworkUnavailable) {
            emit unavailableArtworksFound();
        } else if (anyVectorUnavailable) {
            emit unavailableVectorsFound();
        }
    }

    void ArtItemsModel::onArtworkBackupRequested() {
        LOG_DEBUG << "#";
        ArtworkMetadata *metadata = qobject_cast<ArtworkMetadata *>(sender());
        Q_ASSERT(metadata != nullptr);
        if (metadata != NULL) {
            xpiks()->saveArtworkBackup(metadata);
        }
    }

    void ArtItemsModel::onArtworkEditingPaused() {
        LOG_DEBUG << "#";
        ArtworkMetadata *artwork = qobject_cast<ArtworkMetadata *>(sender());
        Q_ASSERT(artwork != nullptr);
        if (artwork != NULL) {
            xpiks()->submitItemForSpellCheck(artwork->getBasicModel());
        }
    }

    void ArtItemsModel::onArtworkSpellingInfoUpdated() {
        LOG_INTEGR_TESTS_OR_DEBUG << "#";
        ArtworkMetadata *artwork = qobject_cast<ArtworkMetadata *>(sender());
        Q_ASSERT(artwork != nullptr);
        if (artwork != NULL) {
            xpiks()->submitForWarningsCheck(artwork, Common::WarningsCheckFlags::Spelling);
        }
    }

    void ArtItemsModel::onUndoStackEmpty() {
        LOG_DEBUG << "#";
        if (m_ArtworkList.empty()) {
            if (!m_FinalizationList.empty()) {
                LOG_DEBUG << "Clearing the finalization list";
                for (auto *item: m_FinalizationList) {
                    item->deepDisconnect();
                    item->deleteLater();
                }
                m_FinalizationList.clear();
            }
        }
    }

    void ArtItemsModel::userDictUpdateHandler(const QStringList &keywords, bool overwritten) {
        LOG_DEBUG << "#";
        LOG_INTEGRATION_TESTS << keywords;
        size_t size = m_ArtworkList.size();

        Q_ASSERT(!keywords.isEmpty());

        std::vector<Common::BasicKeywordsModel *> itemsToCheck;
        itemsToCheck.reserve(size);

        for (size_t i = 0; i < size; i++) {
            ArtworkMetadata *metadata = accessArtwork(i);
            auto *metadataModel = metadata->getBasicModel();
            SpellCheck::SpellCheckItemInfo *info = metadataModel->getSpellCheckInfo();
            if (!overwritten) {
                info->removeWordsFromErrors(keywords);
            } else {
                info->clear();
            }

            itemsToCheck.push_back(metadataModel);
        }

        if (!overwritten) {
            xpiks()->submitForSpellCheck(itemsToCheck, keywords);
        } else {
            xpiks()->submitForSpellCheck(itemsToCheck);
        }
    }

    void ArtItemsModel::userDictClearedHandler() {
        size_t size = m_ArtworkList.size();
        std::vector<Common::BasicKeywordsModel *> itemsToCheck;
        itemsToCheck.reserve(size);

        for (size_t i = 0; i < size; i++) {
            ArtworkMetadata *metadata = accessArtwork(i);
            auto *keywordsModel = metadata->getBasicModel();
            itemsToCheck.push_back(keywordsModel);
        }

        xpiks()->submitForSpellCheck(itemsToCheck);
    }

    void ArtItemsModel::removeItemsFromRanges(const QVector<QPair<int, int> > &ranges) {
        AbstractListModel::removeItemsFromRanges(ranges);
        syncArtworksIndices();
        emit artworksChanged(true);
    }

    void ArtItemsModel::beginAccountingFiles(int filesCount) {
        int rowsCount = rowCount();
        beginInsertRows(QModelIndex(), rowsCount, rowsCount + filesCount - 1);
    }

    void ArtItemsModel::beginAccountingFiles(int start, int end) {
        beginInsertRows(QModelIndex(), start, end);
    }

    void ArtItemsModel::endAccountingFiles() {
        endInsertRows();
    }

    void ArtItemsModel::beginAccountingManyFiles() {
        beginResetModel();
    }

    void ArtItemsModel::endAccountingManyFiles() {
        endResetModel();
    }

    void ArtItemsModel::syncArtworksIndices() {
        const size_t size = m_ArtworkList.size();
        for (size_t i = 0; i < size; i++) {
            accessArtwork(i);
        }
    }

    void ArtItemsModel::insertArtwork(int index, ArtworkMetadata *artwork) {
        Q_ASSERT(index >= 0 && index <= getArtworksCount());
        Q_ASSERT(artwork != NULL);
        m_ArtworkList.insert(m_ArtworkList.begin() + index, artwork);
        artwork->setCurrentIndex(index);
    }

    void ArtItemsModel::appendArtwork(ArtworkMetadata *artwork) {
        Q_ASSERT(artwork != NULL);
        m_ArtworkList.push_back(artwork);
        artwork->setCurrentIndex(m_ArtworkList.size() - 1);
        LOG_INTEGRATION_TESTS << "Added file:" << artwork->getFilepath();
    }

    ArtworkMetadata *ArtItemsModel::getArtwork(size_t index) const {
        ArtworkMetadata *result = NULL;

        if (index < m_ArtworkList.size()) {
            result = accessArtwork(index);
        }

        return result;
    }

    void ArtItemsModel::raiseArtworksAdded(int importID, int imagesCount, int vectorsCount) {
        // if there're no images added, then we've only attached vectors and no import took place
        Q_ASSERT((imagesCount > 0) || (importID == 0));

        emit artworksAdded(importID, imagesCount, vectorsCount);
        QCoreApplication::processEvents(QEventLoop::AllEvents);

        LOG_INFO << "import #" << importID << "images:" << imagesCount << "vectors:" << vectorsCount;
    }

    void ArtItemsModel::raiseArtworksReimported(int importID, int artworksCount) {
        emit artworksReimported(importID, artworksCount);
        QCoreApplication::processEvents(QEventLoop::AllEvents);

        LOG_INFO << "import #" << importID << "artworks:" << artworksCount;
    }

    void ArtItemsModel::raiseArtworksChanged(bool navigateToCurrent) {
        emit artworksChanged(navigateToCurrent);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }

    void ArtItemsModel::updateItemsAtIndices(const QVector<int> &indices) {
        QVector<int> roles;
        fillStandardRoles(roles);
        updateItemsAtIndicesEx(indices, roles);
    }

    void ArtItemsModel::updateItemsAtIndicesEx(const QVector<int> &indices, const QVector<int> &roles) {        
        LOG_INFO << "Updating roles" << roles << "in" << indices.size() << "item(s)";
        QVector<int> sortedIndices(indices);
        qSort(sortedIndices);
        QVector<QPair<int, int> > ranges;
        Helpers::indicesToRanges(sortedIndices, ranges);
        updateItemsInRangesEx(ranges, roles);
    }

    void ArtItemsModel::updateItemsInRanges(const QVector<QPair<int, int> > &ranges) {
        QVector<int> roles;
        fillStandardRoles(roles);
        updateItemsInRangesEx(ranges, roles);
    }

    void ArtItemsModel::updateItemsInRangesEx(const QVector<QPair<int, int> > &ranges, const QVector<int> &roles) {
        AbstractListModel::updateItemsInRanges(ranges, roles);
    }

    void ArtItemsModel::setAllItemsSelected(bool selected) {
        LOG_DEBUG << selected;
        size_t length = getArtworksCount();

        for (size_t i = 0; i < length; ++i) {
            ArtworkMetadata *metadata = accessArtwork(i);
            metadata->setIsSelected(selected);
        }

        if (length > 0) {
            QModelIndex startIndex = index(0);
            QModelIndex endIndex = index((int)length - 1);
            emit dataChanged(startIndex, endIndex, QVector<int>() << IsSelectedRole);
        }
    }

    int ArtItemsModel::attachVectors(const QHash<QString, QHash<QString, QString> > &vectorsPaths, QVector<int> &indicesToUpdate) const {
        LOG_DEBUG << "#";
        if (vectorsPaths.isEmpty()) { return 0; }

        int attachedVectors = 0;
        QString defaultPath;
        Models::ArtworksRepository *artworksRepository = m_CommandManager->getArtworksRepository();

        size_t size = getArtworksCount();
        indicesToUpdate.reserve((int)size);

        for (size_t i = 0; i < size; ++i) {
            ArtworkMetadata *metadata = accessArtwork(i);
            ImageArtwork *image = dynamic_cast<ImageArtwork *>(metadata);
            if (image == NULL) {
                continue;
            }

            const QString &filepath = image->getFilepath();
            QFileInfo fi(filepath);

            const QString &directory = fi.absolutePath();
            if (vectorsPaths.contains(directory)) {
                const QHash<QString, QString> &innerHash = vectorsPaths[directory];

                const QString &filename = fi.baseName().toLower();

                QString vectorsPath = innerHash.value(filename, defaultPath);
                if (!vectorsPath.isEmpty()) {
                    image->attachVector(vectorsPath);
                    artworksRepository->accountVector(vectorsPath);
                    indicesToUpdate.append((int)i);
                    attachedVectors++;
                }
            }
        }

        LOG_INFO << "Found matches to" << attachedVectors << "file(s)";

        return attachedVectors;
    }

    void ArtItemsModel::unlockAllForIO() {
        LOG_DEBUG << "#";
        size_t size = getArtworksCount();

        for (size_t i = 0; i < size; ++i) {
            ArtworkMetadata *artwork = accessArtwork(i);
            artwork->setIsLockedIO(false);
        }
    }

    void ArtItemsModel::resetSpellCheckResults() {
        LOG_DEBUG << "#";
        const size_t size = m_ArtworkList.size();
        for (size_t i = 0; i < size; i++) {
            ArtworkMetadata *artwork = accessArtwork(i);
            artwork->resetSpellingInfo();
        }
    }

    void ArtItemsModel::resetDuplicatesInfo() {
        LOG_DEBUG << "#";
        const size_t size = m_ArtworkList.size();
        for (size_t i = 0; i < size; i++) {
            ArtworkMetadata *artwork = accessArtwork(i);
            artwork->resetDuplicatesInfo();
        }
    }

    void ArtItemsModel::processUpdateRequests(const std::vector<std::shared_ptr<QMLExtensions::ArtworkUpdateRequest> > &updateRequests) {
        LOG_INFO << updateRequests.size() << "requests to process";

        QVector<int> indicesToUpdate;
        indicesToUpdate.reserve((int)updateRequests.size());
        QSet<int> rolesToUpdateSet;
        int cacheMisses = 0;

        for (auto &request: updateRequests) {
            size_t index = request->getLastKnownIndex();
            auto *artwork = getArtwork(index);

            if ((artwork != nullptr) && (artwork->getItemID() == request->getArtworkID())) {
                indicesToUpdate << (int)index;
                rolesToUpdateSet.unite(request->getRolesToUpdate());
            } else {
                LOG_INTEGRATION_TESTS << "Cache miss. Found" << (artwork ? artwork->getItemID() : -1) << "instead of" << request->getArtworkID();
                request->setCacheMiss();
                cacheMisses++;
            }
        }

        LOG_INFO << cacheMisses << "cache misses out of" << updateRequests.size();

        QVector<int> rolesToUpdate = rolesToUpdateSet.toList().toVector();
        this->updateItemsAtIndicesEx(indicesToUpdate, rolesToUpdate);
    }

    void ArtItemsModel::updateArtworks(const QSet<qint64> &artworkIDs, const QVector<int> &rolesToUpdate) {
        LOG_INFO << artworkIDs.size() << "artworks to find by IDs";
        if (artworkIDs.isEmpty()) { return; }

        QVector<int> indicesToUpdate;
        indicesToUpdate.reserve(artworkIDs.size());

        int i = 0;
        for (auto *artwork: m_ArtworkList) {
            if (artworkIDs.contains(artwork->getItemID())) {
                indicesToUpdate << i;
            }

            i++;
        }

        updateItemsAtIndicesEx(indicesToUpdate, rolesToUpdate);
    }

    Common::IBasicArtwork *ArtItemsModel::getBasicArtwork(int index) const {
        Common::IBasicArtwork *result = NULL;

        if (0 <= index && index < getArtworksCount()) {
            result = accessArtwork(index);
        }

        return result;
    }

    void ArtItemsModel::updateItemAtIndex(int metadataIndex) {
        QVector<int> roles;
        fillStandardRoles(roles);
        QModelIndex topLeft = this->index(metadataIndex);
        QModelIndex bottomRight = this->index(metadataIndex);
        emit dataChanged(topLeft, bottomRight, roles);
    }

    void ArtItemsModel::doCombineArtwork(int index) {
        LOG_DEBUG << "index" << index;
        if (0 <= index && index < getArtworksCount()) {
            ArtworkMetadata *metadata = accessArtwork(index);
            // QModelIndex qmIndex = this->index(index);
            // emit dataChanged(qmIndex, qmIndex, QVector<int>() << IsSelectedRole);

            xpiks()->combineArtwork(metadata, index);
        }
    }

    ArtworkMetadata *ArtItemsModel::accessArtwork(size_t index) const {
        Q_ASSERT(index < m_ArtworkList.size());
        ArtworkMetadata *metadata = m_ArtworkList.at(index);
        metadata->setCurrentIndex(index);
        return metadata;
    }

    QHash<int, QByteArray> ArtItemsModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[ArtworkDescriptionRole] = "description";
        roles[EditArtworkDescriptionRole] = "editdescription";
        roles[ArtworkTitleRole] = "title";
        roles[EditArtworkTitleRole] = "edittitle";
        roles[ArtworkFilenameRole] = "filename";
        roles[KeywordsStringRole] = "keywordsstring";
        roles[IsModifiedRole] = "ismodified";
        roles[IsSelectedRole] = "isselected";
        roles[EditIsSelectedRole] = "editisselected";
        roles[KeywordsCountRole] = "keywordscount";
        roles[HasVectorAttachedRole] = "hasvectorattached";
        roles[BaseFilenameRole] = "basefilename";
        roles[ArtworkThumbnailRole] = "thumbpath";
        roles[IsVideoRole] = "isvideo";
        roles[IsReadOnlyRole] = "isreadonly";
        return roles;
    }

    void ArtItemsModel::destroyInnerItem(ArtworkMetadata *artwork) {
        if (artwork->release()) {
            LOG_INTEGRATION_TESTS << "Destroying metadata" << artwork->getItemID() << "for real";

            bool disconnectStatus = QObject::disconnect(artwork, 0, this, 0);
            if (disconnectStatus == false) { LOG_DEBUG << "Disconnect Artwork from ArtItemsModel returned false"; }
            disconnectStatus = QObject::disconnect(this, 0, artwork, 0);
            if (disconnectStatus == false) { LOG_DEBUG << "Disconnect ArtItemsModel from Artwork returned false"; }

            artwork->deepDisconnect();
            artwork->clearSpellingInfo();
#ifdef QT_DEBUG
            m_DestroyedList.push_back(artwork);
#else
            artwork->deleteLater();
#endif
        } else {
            LOG_DEBUG << "Metadata #" << artwork->getItemID() << "is locked. Postponing destruction...";

            artwork->disconnect();
            auto *metadataModel = artwork->getBasicModel();
            metadataModel->disconnect();
            metadataModel->clearModel();

            m_FinalizationList.push_back(artwork);
        }
    }

    void ArtItemsModel::doRemoveItemsFromRanges(QVector<int> &indicesToRemove, bool isFullDirectory) {
        qSort(indicesToRemove);
        QVector<QPair<int, int> > rangesToRemove;
        Helpers::indicesToRanges(indicesToRemove, rangesToRemove);
        doRemoveItemsInRanges(rangesToRemove, isFullDirectory);
    }

    void ArtItemsModel::doRemoveItemsInRanges(const QVector<QPair<int, int> > &rangesToRemove, bool isFullDirectory) {
        std::shared_ptr<Commands::RemoveArtworksCommand> removeArtworksCommand(new Commands::RemoveArtworksCommand(rangesToRemove, isFullDirectory));

        m_CommandManager->processCommand(removeArtworksCommand);
    }

    void ArtItemsModel::getSelectedItemsIndices(QVector<int> &indices) {
        size_t size = m_ArtworkList.size();

        indices.reserve((int)size / 3);
        for (size_t i = 0; i < size; ++i) {
            if (accessArtwork(i)->isSelected()) {
                indices.append((int)i);
            }
        }
    }

    QVector<int> ArtItemsModel::getArtworkStandardRoles() const {
        QVector<int> roles;
        fillStandardRoles(roles);
        return roles;
    }

    void ArtItemsModel::fillStandardRoles(QVector<int> &roles) const {
        roles << ArtworkDescriptionRole << IsModifiedRole <<
            ArtworkTitleRole << KeywordsCountRole << HasVectorAttachedRole;
    }

#ifdef INTEGRATION_TESTS
    ArtworkMetadata *ArtItemsModel::findArtworkByFilepath(const QString &filepath) {
        const size_t size = m_ArtworkList.size();
        for (size_t i = 0; i < size; i++) {
            ArtworkMetadata *metadata = getArtwork(i);
            if (metadata->getFilepath() == filepath) {
                return metadata;
            }
        }
        return nullptr;
    }

#endif
}
