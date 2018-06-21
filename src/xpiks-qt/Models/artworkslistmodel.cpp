/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworkslistmodel.h"
#include <QDir>
#include <QtQml>
#include "../Artworks/basickeywordsmodel.h"
#include "../Artworks/imageartwork.h"
#include "../Artworks/videoartwork.h"
#include "artworksrepository.h"
#include "../Helpers/artworkshelpers.h"
#include "../Helpers/stringhelper.h"
#include "../Commands/icommandmanager.h"
#include "../KeywordsPresets/ipresetsmanager.h"
#include "../Artworks/iartworksservice.h"
#include "../Commands/expandpresetcommand.h"
#include "../AutoComplete/icompletionsource.h"

namespace Models {
    ArtworksListModel::ArtworksListModel(ArtworksRepository &repository,
                                         Commands::ICommandManager &commandManager,
                                         KeywordsPresets::IPresetsManager &presetsManager,
                                         Artworks::IArtworksService &inspectionService,
                                         AutoComplete::ICompletionSource &completionSource,
                                         QObject *parent):
        QAbstractListModel(parent),
        // all items before 1024 are reserved for internal models
        m_LastID(1024),
        m_CurrentItemIndex(0),
        m_ArtworksRepository(repository),
        m_CommandManager(commandManager),
        m_PresetsManager(presetsManager),
        m_InspectionService(inspectionService),
        m_CompletionSource(completionSource)
    {
        QObject::connect(&m_ArtworksRepository, &ArtworksRepository::filesUnavailable,
                         this, &ArtworksListModel::onFilesUnavailableHandler);
    }

    ArtworksListModel::~ArtworksListModel() {
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

    int ArtworksListModel::getModifiedArtworksCount() {
        int modifiedCount = 0;
        foreachArtwork([](ArtworkMetadata *artwork) { return
                    !artwork->isUnavailable() &&
                    !artwork->isRemoved() &&
                    artwork->isModified() },
        [&modifiedCount](ArtworkMetadata *, size_t) { modifiedCount++ });
        return modifiedCount;
    }

    QVector<int> ArtworksListModel::getArtworkStandardRoles() const {
        return QVector<int>() << ArtworkDescriptionRole << IsModifiedRole <<
                                 ArtworkTitleRole << KeywordsCountRole << HasVectorAttachedRole;
    }

    void ArtworksListModel::selectArtworksFromDirectory(int directoryIndex) {
        LOG_INFO << "Select directory at" << directoryIndex;
        const QString &directory = m_ArtworksRepository.getDirectoryPath(directoryIndex);
        LOG_FOR_TESTS << "Select directory:" << directory;

        const QString directoryAbsolutePath = QDir(directory).absolutePath();
        std::vector<int> indices = this->selectArtworks(
                    [&directoryAbsolutePath](Artworks::ArtworkMetadata *artwork) {
                return artwork->isInDirectory(directoryAbsolutePath); },
                [](Artworks::ArtworkMetadata *, size_t index) { return (int)index; });

        Helpers::IndicesRanges ranges(indices);

        this->foreachArtwork(ranges, [](Artworks::ArtworkMetadata *artwork, size_t) {
            artwork->setIsSelected(true); });
        this->updateItems(ranges, QVector<int>() << IsSelectedRole);
    }

    void ArtworksListModel::updateItems(const Helpers::IndicesRanges &ranges, const QVector<int> &roles) {
        for (auto &r: ranges.getRanges()) {
            QModelIndex topLeft = index(r.first);
            QModelIndex bottomRight = index(r.second);
            emit dataChanged(topLeft, bottomRight, roles);
        }
    }

    void ArtworksListModel::updateItems(ArtworksListModel::SelectionType selectionType, const QVector<int> &roles) {
        std::function<bool (Artworks::ArtworkMetadata *)> pred;
        switch (selectionType) {
        case SelectionType::All:
            this->updateItems(Helpers::IndicesRanges(getArtworksCount()), roles);
            return;
        case SelectionType::Modified:
            pred = [](Artworks::ArtworkMetadata *artwork) { return artwork->isModified(); };
            break;
        case SelectionType::Selected:
            pred = [](Artworks::ArtworkMetadata *artwork) { return artwork->isSelected(); };
            break;
        default:
            return;
        }

        auto indices = selectArtworks(pred, [](Artworks::ArtworkMetadata*, size_t i) { return (int)i; });
        this->updateItems(Helpers::IndicesRanges(indices), roles);
    }

    std::unique_ptr<Artworks::SessionSnapshot> ArtworksListModel::snapshotAll() {
        std::unique_ptr<Artworks::SessionSnapshot> sessionSnapshot(
                    new Artworks::SessionSnapshot(
                        selectAvailableArtworks([](ArtworkMetadata *artwork, size_t) {return artwork;}),
                        m_ArtworksRepository.retrieveFullDirectories()));
        return sessionSnapshot;
    }

    void ArtworksListModel::generateAboutToBeRemoved() {
        LOG_DEBUG << "#";
        foreachArtwork([](ArtworkMetadata *artwork) { return artwork->isUnavailable(); },
        [](ArtworkMetadata *artwork, size_t) {
            Artworks::BasicKeywordsModel *keywordsModel = artwork->getBasicModel();
            keywordsModel->notifyAboutToBeRemoved();
        });
    }

    void ArtworksListModel::unlockAllForIO() {
        LOG_DEBUG << "#";
        foreachArtwork([](ArtworkMetadata *){return true;},
        [](ArtworkMetadata *artwork, size_t) { artwork->setIsLockedIO(false); });
    }

    void ArtworksListModel::processUpdateRequests(const std::vector<std::shared_ptr<QMLExtensions::ArtworkUpdateRequest> > &updateRequests) {
        LOG_INFO << updateRequests.size() << "requests to process";

        std::vector<int> indicesToUpdate;
        indicesToUpdate.reserve(updateRequests.size());
        QSet<int> rolesToUpdateSet;
        int cacheMisses = 0;

        for (auto &request: updateRequests) {
            size_t index = request->getLastKnownIndex();
            auto *artwork = getArtwork(index);
            if (artwork->getItemID() == request->getArtworkID()) {
                indicesToUpdate.push_back((int)index);
                rolesToUpdateSet.unite(request->getRolesToUpdate());
            } else {
                LOG_INTEGRATION_TESTS << "Cache miss. Found" << (artwork ? artwork->getItemID() : -1) << "instead of" << request->getArtworkID();
                request->setCacheMiss();
                cacheMisses++;
            }
        }

        LOG_INFO << cacheMisses << "cache misses out of" << updateRequests.size();

        QVector<int> rolesToUpdate = rolesToUpdateSet.toList().toVector();
        this->updateItems(Helpers::IndicesRanges(indicesToUpdate), rolesToUpdate);
    }

    void ArtworksListModel::updateArtworksByIDs(const QSet<qint64> &artworkIDs, const QVector<int> &rolesToUpdate) {
        LOG_INFO << artworkIDs.size() << "artworks to find by IDs";
        if (artworkIDs.isEmpty()) { return; }

        std::vector<int> indices = selectArtworks(
                    [&artworkIDs](ArtworkMetadata *artwork) { return artworkIDs.contains(artwork->getItemID()); },
                [](ArtworkMetadata *, size_t index) { return (int)index; });

        this->updateItems(Helpers::IndicesRanges(indices, rolesToUpdate));
    }

    ArtworksListModel::ArtworksAddResult ArtworksListModel::addFiles(const std::shared_ptr<Filesystem::IFilesCollection> &filesCollection,
                                                                     Common::AddFilesFlags flags) {
        const int newFilesCount = m_ArtworksRepository.getNewFilesCount(filesCollection);
        Artworks::ArtworksSnapshot snapshot;
        snapshot.reserve(newFilesCount);
        qint64 directoryID = 0;
        const int count = getArtworksCount();

        emit beginInsertRows(QModelIndex(), count, count + newFilesCount - 1);
        {
            for (auto &file: filesCollection->getFiles()) {
                if (file.m_Type == Filesystem::ArtworkFileType::Vector) { continue; }

                if (m_ArtworksRepository.accountFile(file.m_Path, directoryID, directoryFlags)) {
                    ArtworkMetadata *artwork = createArtwork(file, directoryID);
                    appendArtwork(artwork);
                    connectArtworkSignals(artwork);
                }
            }
        }
        emit endInsertRows();

        const bool autoAttach = Common::HasFlag(flags, Common::AddFilesFlags::FlagAutoFindVectors);
        const int attachedCount = attachVectors(filesCollection, snapshot, count, autoAttach);
        m_ArtworksRepository.addFiles(snapshot);

        // TODO: fix this:
        //syncArtworksIndices();

        return {
            snapshot,
            attachedCount
        };
    }

    ArtworksListModel::ArtworksRemoveResult ArtworksListModel::removeFiles(const Helpers::IndicesRanges &ranges) {
        int selectedCount = 0;
        foreachArtwork(ranges, [this, &selectedCount](ArtworkMetadata *artwork, size_t) {
            artwork->setRemoved();
            if (artwork->isSelected()) {
                artwork->resetSelected();
                selectedCount++;
            }
        });

        if (selectedCount > 0) {
            emit selectedArtworksRemoved(selectedCount);
        }

        emit modifiedArtworksCountChanged();
        emit artworksChanged(true);

        Artworks::WeakArtworksSnapshot snapshot = selectArtworks(
                    [](ArtworkMetadata *artwork) { return artwork->isRemoved(); },
                [](ArtworkMetadata *artwork, size_t) { return artwork; });

        auto removeResult = m_ArtworksRepository.removeFiles(snapshot);
        return {
            std::get<0>(removeResult), // directories ids set
                    snapshot.size(), // removed count
            std::get<1>(removeResult) // unselect all
        };
    }

    ArtworksListModel::ArtworksRemoveResult ArtworksListModel::removeFilesFromDirectory(int directoryIndex) {
        LOG_INFO << "Remove artworks directory at" << directoryIndex;
        const QString &directory = m_ArtworksRepository.getDirectoryPath(directoryIndex);
        LOG_FOR_TESTS << "Removing directory:" << directory;

        const QString directoryAbsolutePath = QDir(directory).absolutePath();
        std::vector<int> indices = this->selectArtworks(
                    [&directoryAbsolutePath](ArtworkMetadata *artwork) {
                return artwork->isInDirectory(directoryAbsolutePath); },
                [](ArtworkMetadata *, size_t index) { return (int)index; });

        return removeFiles(Helpers::IndicesRanges(indices));
    }

    void ArtworksListModel::restoreRemoved() {
        LOG_DEBUG << "#";
        foreachArtwork([](ArtworkMetadata *artwork) { return artwork->isRemoved(); },
        [](ArtworkMetadata *artwork, size_t) {
            m_ArtworksRepository.accountFile(artwork->getFilepath(), artwork->getDirectoryID(), directoryFlags);
        });
        // TODO: finish this
    }

    void ArtworksListModel::deleteRemovedItems() {
        LOG_DEBUG << "#";
        std::vector<int> indices = this->selectArtworks(
                    [](ArtworkMetadata *artwork){ return artwork->isRemoved(); },
                [](ArtworkMetadata *, size_t index) { return (int)index; });
        deleteItems(Helpers::IndicesRanges(indices));
    }

    void ArtworksListModel::deleteUnavailableItems() {
        LOG_DEBUG << "#";
        std::vector<int> indices = this->selectArtworks(
                    [](ArtworkMetadata *artwork){ return artwork->isUnavailable(); },
                [](ArtworkMetadata *, size_t index) { return (int)index; });
        deleteItems(Helpers::IndicesRanges(indices));
    }

    void ArtworksListModel::deleteAllItems() {
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

        for (auto *artwork: artworksToDestroy) {
            destroyArtwork(artwork);
        }
    }

    Artworks::ArtworksSnapshot ArtworksListModel::deleteItems(const Helpers::IndicesRanges &ranges) {
        LOG_DEBUG << ranges.size() << "range(s)";
        QModelIndex dummy;
        const bool willReset = ranges.length() > 20;
        if (willReset) { emit beginResetModel(); }
        auto &rangesArray = ranges.getRanges();
        const int size = (int)rangesArray.size();

        for (int i = size - 1; i >= 0; i--) {
            auto &r = rangesArray.at(i);
            Q_ASSERT(r.first >= 0 && r.first < getArtworksCount());
            Q_ASSERT(r.second >= 0 && r.second < getArtworksCount());

            auto itBegin = m_ArtworkList.begin() + r.first;
            auto itEnd = m_ArtworkList.begin() + (r.second + 1);

            std::vector<ArtworkMetadata *> itemsToDelete(itBegin, itEnd);
            if (!willReset) { emit beginRemoveRows(dummy, r.first, r.last); }
            {
                m_ArtworkList.erase(itBegin, itEnd);
            }
            if (!willReset) { emit endRemoveRows(); }

            std::vector<ArtworkMetadata *>::iterator it = itemsToDelete.begin();
            std::vector<ArtworkMetadata *>::iterator itemsEnd = itemsToDelete.end();
            for (; it < itemsEnd; it++) {
                ArtworkMetadata *artwork = *it;
                Q_ASSERT(artwork->isRemoved());
                m_ArtworksRepository.removeFile(artwork->getFilepath(), artwork->getDirectoryID());
                LOG_INTEGRATION_TESTS << "File removed:" << artwork->getFilepath();
                destroyArtwork(artwork);
            }
        }

        if (willReset) { emit endResetModel(); }
        syncArtworksIndices();
    }

    int ArtworksListModel::attachVectors(const std::shared_ptr<Filesystem::IFilesCollection> &filesCollection,
                                         const Artworks::ArtworksSnapshot &snapshot,
                                         int initialCount,
                                         bool autoAttach) {
        QHash<QString, QHash<QString, QString> > vectors;
        for (auto &file: filesCollection->getFiles()) {
            if (file.m_Type != Filesystem::ArtworkFileType::Vector) { continue; }

            QFileInfo fi(file.m_Path);
            const QString &absolutePath = fi.absolutePath();

            if (!vectors.contains(absolutePath)) {
                vectors.insert(absolutePath, QHash<QString, QString>());
            }

            vectors[absolutePath].insert(fi.baseName().toLower(), path);
        }

        QVector<int> indicesToUpdate;
        int attachedCount = attachKnownVectors(vectors, indicesToUpdate);

        if (autoAttach) {
            QVector<int> autoAttachedIndices;
            attachedCount += Helpers::findAndAttachVectors(snapshot.getWeakSnapshot(), autoAttachedIndices);

            foreach (int index, autoAttachedIndices) {
                indicesToUpdate.append(initialCount + index);
            }
        }

        updateItems(Helpers::IndicesRanges(indicesToUpdate),
                    QVector<int>() << Models::ArtworksListModel::HasVectorAttachedRole);
        return attachedCount;
    }

    int ArtworksListModel::attachKnownVectors(const QHash<QString, QHash<QString, QString> > &vectorsPaths,
                                              QVector<int> &indicesToUpdate) const {
        LOG_DEBUG << "#";
        if (vectorsPaths.isEmpty()) { return 0; }

        int attachedVectors = 0;
        QString defaultPath;

        const size_t size = getArtworksCount();
        indicesToUpdate.reserve((int)size);

        for (size_t i = 0; i < size; ++i) {
            ArtworkMetadata *metadata = accessArtwork(i);
            Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork *>(metadata);
            if (image == NULL) { continue; }

            const QString &filepath = image->getFilepath();
            QFileInfo fi(filepath);

            const QString &directory = fi.absolutePath();
            if (vectorsPaths.contains(directory)) {
                const QHash<QString, QString> &innerHash = vectorsPaths[directory];

                const QString &filename = fi.baseName().toLower();

                QString vectorsPath = innerHash.value(filename, defaultPath);
                if (!vectorsPath.isEmpty()) {
                    image->attachVector(vectorsPath);
                    indicesToUpdate.append((int)i);
                    attachedVectors++;
                }
            }
        }

        LOG_INFO << "Found matches to" << attachedVectors << "file(s)";

        return attachedVectors;
    }

    void ArtworksListModel::connectArtworkSignals(Artworks::ArtworkMetadata *artwork) {
        QObject::connect(artwork, &Artworks::ArtworkMetadata::modifiedChanged,
                         this, &ArtworksListModel::itemModifiedChanged);

        QObject::connect(artwork, &Artworks::ArtworkMetadata::backupRequired,
                         this, &ArtworksListModel::onArtworkBackupRequested);

        QObject::connect(artwork, &Artworks::ArtworkMetadata::editingPaused,
                         this, &ArtworksListModel::onArtworkEditingPaused);

        QObject::connect(artwork, &Artworks::ArtworkMetadata::spellingInfoUpdated,
                         this, &ArtworksListModel::onArtworkSpellingInfoUpdated);

        QObject::connect(artwork, &Artworks::ArtworkMetadata::selectedChanged,
                         this, &ArtworksListModel::artworkSelectedChanged);
    }

    void ArtworksListModel::syncArtworksIndices() {
        foreachArtwork([](ArtworkMetadata *){return true;},
        [](ArtworkMetadata *, size_t i) { accessArtwork(i); });
    }

    QVariant ArtworksListModel::data(const QModelIndex &index, int role) const {
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
                Artworks::BasicKeywordsModel *keywordsModel = artwork->getBasicModel();
                return keywordsModel->getKeywordsString();
            }
            case IsModifiedRole:
                return artwork->isModified();
            case IsSelectedRole:
                return artwork->isSelected();
            case KeywordsCountRole: {
                Artworks::BasicKeywordsModel *keywordsModel = artwork->getBasicModel();
                return keywordsModel->getKeywordsCount();
            }
            case HasVectorAttachedRole: {
                Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork *>(artwork);
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

    Qt::ItemFlags ArtworksListModel::flags(const QModelIndex &index) const {
        int row = index.row();

        if (row < 0 || row >= getArtworksCount()) {
            return Qt::ItemIsEnabled;
        }

        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    bool ArtworksListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
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

    ArtworkMetadata *ArtworksListModel::getBasicModelObject(int index) const {
        Artworks::ArtworkMetadata *item = NULL;

        if (0 <= index && index < getArtworksCount()) {
            item = accessArtwork(index);
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        }

        return item;
    }

    Artworks::BasicMetadataModel *ArtworksListModel::getBasicModelObject(int index) const {
        Artworks::BasicMetadataModel *keywordsModel = NULL;

        if (0 <= index && index < getArtworksCount()) {
            keywordsModel = accessArtwork(index)->getBasicModel();
            QQmlEngine::setObjectOwnership(keywordsModel, QQmlEngine::CppOwnership);
        }

        return keywordsModel;
    }

    bool ArtworksListModel::removeKeywordAt(int artworkIndex, int keywordIndex) {
        LOG_INFO << "metadata index" << artworkIndex << "| keyword index" << keywordIndex;
        bool success = false;
        ArtworkMetadata *artwork = getArtwork(artworkIndex);
        if (artwork != nullptr) {
            setCurrentIndex(artworkIndex);
            QString removed;
            if (artwork->removeKeywordAt(keywordIndex, removed)) {
                success = true;
                QModelIndex index = this->index(artworkIndex);
                emit dataChanged(index, index, QVector<int>() << IsModifiedRole << KeywordsCountRole);
                m_InspectionService.submitArtwork(artwork);
            }
        }
        return success;
    }

    bool ArtworksListModel::removeLastKeyword(int artworkIndex) {
        LOG_INFO << "index" << artworkIndex;
        bool success = false;
        ArtworkMetadata *artwork = accessArtwork(artworkIndex);
        if (artwork != nullptr) {
            setCurrentIndex(artworkIndex);
            QString removed;
            if (artwork->removeLastKeyword(removed)) {
                success = true;
                QModelIndex index = this->index(artworkIndex);
                emit dataChanged(index, index, QVector<int>() << IsModifiedRole << KeywordsCountRole);
                m_InspectionService.submitArtwork(artwork);
            }
        }
        return success;
    }

    bool ArtworksListModel::appendKeyword(int artworkIndex, const QString &keyword) {
        bool added = false;
        LOG_INFO << "metadata index" << artworkIndex << "| keyword" << keyword;

        ArtworkMetadata *artwork = getArtwork(artworkIndex);
        if (artwork != nullptr) {
            setCurrentIndex(artworkIndex);
            if (artwork->appendKeyword(keyword)) {
                added = true;
                QModelIndex index = this->index(artworkIndex);
                emit dataChanged(index, index, QVector<int>() << IsModifiedRole << KeywordsCountRole);
                m_InspectionService.submitArtwork(artwork);
            }
        }

        return added;
    }

    void ArtworksListModel::pasteKeywords(int artworkIndex, const QStringList &keywords) {
        LOG_INFO << "item index" << artworkIndex << "|" << keywords;
        if (artworkIndex >= 0
            && artworkIndex < getArtworksCount()
            && !keywords.empty()) {
            setCurrentIndex(artworkIndex);
            Artworks::ArtworksSnapshot::Container rawArtworkSnapshot;
            QVector<int> selectedIndices;

            // TODO: to be changed in future to the dialog
            // getSelectedItemsIndices(selectedIndices);
            // if (!metadata->getIsSelected()) {
            selectedIndices.append(artworkIndex);
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
            m_CommandManager.processCommand(pasteCommand);
        }
    }

    void ArtworksListModel::addSuggestedKeywords(int artworkIndex, const QStringList &keywords) {
        LOG_DEBUG << "item index" << artworkIndex;
        ArtworkMetadata *artwork = getArtwork(artworkIndex);
        if (artwork != nullptr && !keywords.empty()) {
            setCurrentIndex(artworkIndex);
            Artworks::ArtworksSnapshot::Container rawSnapshot;
            rawSnapshot.emplace_back(new ArtworkMetadataLocker(artwork));

            std::shared_ptr<Commands::PasteKeywordsCommand> pasteCommand(new Commands::PasteKeywordsCommand(rawSnapshot, keywords));
            m_CommandManager.processCommand(pasteCommand);
        }
    }

    void ArtworksListModel::setItemsSaved(const Helpers::IndicesRanges &ranges) {
        LOG_DEBUG << "#";
        foreachArtwork(ranges, [](ArtworkMetadata *artwork, size_t) {
            artwork->resetModified();
        });

        this->updateItems(ranges, QVector<int>() << IsModifiedRole);

        emit modifiedArtworksCountChanged();
        emit artworksChanged(false);
    }

    bool ArtworksListModel::editKeyword(int artworkIndex, int keywordIndex, const QString &replacement) {
        LOG_INFO << "metadata index:" << artworkIndex;
        bool edited = false;
        ArtworkMetadata *artwork = getArtwork(artworkIndex);
        if (artwork != nullptr) {
            setCurrentIndex(artworkIndex);
            if (artwork->editKeyword(keywordIndex, replacement)) {
                edited = true;
                QModelIndex index = this->index(artworkIndex);
                emit dataChanged(index, index, QVector<int>() << IsModifiedRole << KeywordsCountRole);
                m_InspectionService.submitArtwork(artwork);
            }
        }
        return edited;
    }

    void ArtworksListModel::plainTextEdit(int artworkIndex, const QString &rawKeywords, bool spaceIsSeparator) {
        LOG_DEBUG << "Plain text edit for item" << artworkIndex;
        ArtworkMetadata *artwork = getArtwork(artworkIndex);
        if (artwork != nullptr) {
            setCurrentIndex(artworkIndex);
            QVector<QChar> separators;
            separators << QChar(',');
            if (spaceIsSeparator) { separators << QChar::Space; }
            QStringList keywords;
            Helpers::splitKeywords(rawKeywords.trimmed(), separators, keywords);

            Artworks::ArtworksSnapshot::Container items;
            items.emplace_back(new ArtworkMetadataLocker(artwork));

            Common::CombinedEditFlags flags = Common::CombinedEditFlags::None;
            Common::SetFlag(flags, Common::CombinedEditFlags::EditKeywords);
            std::shared_ptr<Commands::CombinedEditCommand> combinedEditCommand(new Commands::CombinedEditCommand(
                    flags,
                    items,
                    "", "",
                    keywords));

            m_CommandManager.processCommand(combinedEditCommand);
            updateItemAtIndex(artworkIndex);
        }
    }

    void ArtworksListModel::detachVectorsFromArtworks(const Helpers::IndicesRanges &ranges) {
        foreachArtworkAs<Artworks::ImageArtwork>(ranges,
                                       [](Artworks::ImageArtwork *image) { return image->hasVectorAttached(); },
        [this](Artworks::ImageArtwork *image, size_t) {
            this->m_ArtworksRepository.removeVector(
                        image->getAttachedVectorPath());
            image->detachVector();
        });

        updateItems(ranges, QVector<int>() << HasVectorAttachedRole);
    }

    void ArtworksListModel::expandPreset(int artworkIndex, int keywordIndex, unsigned int presetID) {
        LOG_INFO << "item" << artworkIndex << "keyword" << keywordIndex << "preset" << presetID;

        ArtworkMetadata *artwork = getArtwork(artworkIndex);
        if (artwork != nullptr) {
            setCurrentIndex(artworkIndex);
            std::shared_ptr<Commands::ExpandPresetCommand> expandPresetCommand(
                        new Commands::ExpandPresetCommand(artwork,
                                                          (KeywordsPresets::ID_t)presetID,
                                                          keywordIndex));
            auto result = m_CommandManager.processCommand(expandPresetCommand);
            Q_UNUSED(result);
        }
    }

    void ArtworksListModel::expandLastAsPreset(int artworkIndex) {
        LOG_INFO << "item" << artworkIndex;

        ArtworkMetadata *artwork = getArtwork(artworkIndex);
        if (artwork != nullptr) {
            setCurrentIndex(artworkIndex);
            auto *basicModel = artwork->getBasicModel();
            int keywordIndex = basicModel->getKeywordsCount() - 1;
            QString lastKeyword = basicModel->retrieveKeyword(keywordIndex);

            KeywordsPresets::ID_t presetID;
            if (m_PresetsManager.tryFindSinglePresetByName(lastKeyword, false, presetID)) {
                std::shared_ptr<Commands::ExpandPresetCommand> expandPresetCommand(
                            new Commands::ExpandPresetCommand(artwork, presetID, keywordIndex));
                std::shared_ptr<Commands::CommandResult> result = m_CommandManager.processCommand(expandPresetCommand);
                Q_UNUSED(result);
            }
        }
    }

    void ArtworksListModel::addPreset(int artworkIndex, unsigned int presetID) {
        LOG_INFO << "item" << artworkIndex << "preset" << presetID;

        ArtworkMetadata *artwork = getArtwork(artworkIndex);
        if (artwork != nullptr) {
            setCurrentIndex(artworkIndex);
            std::shared_ptr<Commands::ExpandPresetCommand> expandPresetCommand(
                        new Commands::ExpandPresetCommand(artwork, presetID));
            std::shared_ptr<Commands::CommandResult> result = m_CommandManager.processCommand(expandPresetCommand);
            Q_UNUSED(result);
        }
    }

    bool ArtworksListModel::acceptCompletionAsPreset(int artworkIndex, int completionID) {
        LOG_INFO << "item" << artworkIndex << "completionID" << completionID;
        bool accepted = false;

        ArtworkMetadata *artwork = getArtwork(artworkIndex);
        if (artwork != nullptr) {
            setCurrentIndex(artworkIndex);
            std::shared_ptr<AutoComplete::CompletionItem> completionItem = m_CompletionSource.getAcceptedCompletion(completionID);
            if (!completionItem) {
                LOG_WARNING << "Completion is not available anymore";
                return false;
            }

            const int presetID = completionItem->getPresetID();

            if (completionItem->isPreset() ||
                    (completionItem->canBePreset() && completionItem->shouldExpandPreset())) {
                std::shared_ptr<Commands::ExpandPresetCommand> expandPresetCommand(new Commands::ExpandPresetCommand(artwork, presetID));
                std::shared_ptr<Commands::CommandResult> result = m_CommandManager.processCommand(expandPresetCommand);
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

    void ArtworksListModel::onFilesUnavailableHandler() {
        LOG_DEBUG << "#";
        bool anyArtworkUnavailable = false;
        bool anyVectorUnavailable = false;

        foreachArtwork([this](ArtworkMetadata *artwork) {
            return this->m_ArtworksRepository.isFileUnavailable(artwork->getFilepath()); },
        [&anyArtworkUnavailable](ArtworkMetadata *artwork, size_t) {
            artwork->setUnavailable(); anyArtworkUnavailable = true; });

        foreachArtworkAs<Artworks::ImageArtwork>(Helpers::IndicesRanges(getArtworksCount()),
                    [this](Artworks::ImageArtwork *image) { return image->hasVectorAttached() &&
                    this->m_ArtworksRepository.isFileUnavailable(image->getAttachedVectorPath()); },
        [&anyVectorUnavailable](Artworks::ImageArtwork *image, size_t) {
            image->detachVector(); anyVectorUnavailable = true; });

        if (anyArtworkUnavailable) {
            emit unavailableArtworksFound();
        } else if (anyVectorUnavailable) {
            emit unavailableVectorsFound();
        }
    }

    void ArtworksListModel::onUndoStackEmpty() {
        LOG_DEBUG << "#";

        deleteRemovedItems();

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

    QHash<int, QByteArray> ArtworksListModel::roleNames() const {
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

    ArtworkMetadata *ArtworksListModel::createArtwork(const Filesystem::ArtworkFile &file, qint64 directoryID) {
        ArtworkMetadata *artwork = nullptr;
        if (file.m_Type == Filesystem::ArtworkFileType::Image) {
            artwork = new ImageArtwork(file.m_Path, getNextID(), directoryID);
        } else if (file.m_Type == Filesystem::ArtworkFileType::Video) {
            artwork = new VideoArtwork(file, getNextID(), directoryID);
        }
        Q_ASSERT(artwork != nullptr);
        return artwork;
    }

    ArtworkMetadata *ArtworksListModel::accessArtwork(size_t index) const {
        Q_ASSERT(index < m_ArtworkList.size());
        ArtworkMetadata *artwork = m_ArtworkList.at(index);
        artwork->setCurrentIndex(index);
        return artwork;
    }

    ArtworkMetadata *ArtworksListModel::getArtwork(size_t index) const {
        Q_ASSERT(index < m_ArtworkList.size());
        if (index < m_ArtworkList.size()) {
            return accessArtwork(index);
        }

        return nullptr;
    }

    void ArtworksListModel::destroyArtwork(Artworks::ArtworkMetadata *artwork) {
        if (artwork->release()) {
            LOG_INTEGRATION_TESTS << "Destroying metadata" << artwork->getItemID() << "for real";

            bool disconnectStatus = QObject::disconnect(artwork, 0, this, 0);
            if (disconnectStatus == false) { LOG_DEBUG << "Disconnect Artwork from ArtworksListModel returned false"; }
            disconnectStatus = QObject::disconnect(this, 0, artwork, 0);
            if (disconnectStatus == false) { LOG_DEBUG << "Disconnect ArtworksListModel from Artwork returned false"; }

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

    int ArtworksListModel::getNextID() {
        return m_LastID++;
    }

    void ArtworksListModel::foreachArtwork(const Helpers::IndicesRanges &ranges,
                                           std::function<bool (ArtworkMetadata *)> pred,
                                           std::function<void (ArtworkMetadata *, size_t)> action) const {
        for (auto &r: ranges.getRanges()) {
            for (int i = r.first; i <= r.second; r++) {
                auto *artwork = accessArtwork(i);
                if (pred(artwork)) {
                    action(artwork, i);
                }
            }
        }
    }

    void ArtworksListModel::foreachArtwork(std::function<bool (ArtworkMetadata *)> pred,
                                           std::function<void (ArtworkMetadata *, size_t)> action) const {
        foreachArtwork(Helpers::IndicesRanges(0, getArtworksCount()),
                       pred,
                       action);
    }

    void ArtworksListModel::foreachArtwork(const Helpers::IndicesRanges &ranges,
                                           std::function<void (ArtworkMetadata *, size_t)> action) const {
        foreachArtwork(ranges,
                       [](ArtworkMetadata *){ return true; },
                       action);
    }
}
