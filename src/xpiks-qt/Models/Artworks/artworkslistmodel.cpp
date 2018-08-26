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
#include <functional>
#include "artworksrepository.h"
#include <Artworks/basickeywordsmodel.h>
#include <Artworks/imageartwork.h>
#include <Artworks/videoartwork.h>
#include <Helpers/artworkshelpers.h>
#include <Helpers/stringhelper.h>
#include <KeywordsPresets/ipresetsmanager.h>
#include <Commands/Base/icommandmanager.h>
#include <Commands/Editing/expandpresettemplate.h>
#include <Commands/Editing/editartworkstemplate.h>
#include <Commands/Base/templatedcommand.h>
#include <Commands/Editing/modifyartworkscommand.h>
#include <Commands/Base/compositecommandtemplate.h>
#include <Commands/artworksupdatetemplate.h>
#include <Commands/Editing/keywordedittemplate.h>
#include <Commands/Base/emptycommand.h>
#include <Models/Editing/currenteditableartwork.h>
#include <Services/AutoComplete/icompletionsource.h>
#include <Services/artworkupdaterequest.h>

namespace Models {
    void artworkDeleter(Artworks::ArtworkMetadata *artwork) {
        artwork->deleteLater();
    }

    using ArtworksTemplate = Commands::ICommandTemplate<Artworks::ArtworksSnapshot>;
    using ArtworksTemplateComposite = Commands::CompositeCommandTemplate<Artworks::ArtworksSnapshot>;
    using ArtworksCommand = Commands::TemplatedCommand<Artworks::ArtworksSnapshot>;

    ArtworksListModel::ArtworksListModel(ArtworksRepository &repository,
                                         QObject *parent):
        QAbstractListModel(parent),
        // all items before 1024 are reserved for internal models
        m_LastID(1024),
        m_CurrentItemIndex(-1),
        m_ArtworksRepository(repository)
    {
        QObject::connect(&m_ArtworksRepository, &ArtworksRepository::filesUnavailable,
                         this, &ArtworksListModel::onFilesUnavailableHandler);
    }

    ArtworksListModel::~ArtworksListModel() {
        LOG_FOR_TESTS << "Artworks destroyed";
    }

    int ArtworksListModel::getModifiedArtworksCount() {
        int modifiedCount = 0;
        foreachArtwork([](ArtworkItem const &artwork) { return
                    !artwork->isUnavailable() &&
                    !artwork->isRemoved() &&
                    artwork->isModified(); },
        [&modifiedCount](ArtworkItem const &, size_t) { modifiedCount++; });
        return modifiedCount;
    }

    QVector<int> ArtworksListModel::getStandardUpdateRoles() const {
        return QVector<int>() << ArtworkDescriptionRole << IsModifiedRole <<
                                 ArtworkTitleRole << KeywordsCountRole << HasVectorAttachedRole;
    }

    void ArtworksListModel::selectArtworksFromDirectory(int directoryIndex) {
        LOG_INFO << "Select directory at" << directoryIndex;
        const QString &directory = m_ArtworksRepository.getDirectoryPath(directoryIndex);
        LOG_FOR_TESTS << "Select directory:" << directory;

        const QString directoryAbsolutePath = QDir(directory).absolutePath();
        auto indices = this->filterArtworks<int>(
                    [&directoryAbsolutePath](ArtworkItem const &artwork) {
                return artwork->isInDirectory(directoryAbsolutePath); },
                [](ArtworkItem const &, size_t index) { return (int)index; });

        Helpers::IndicesRanges ranges(indices);

        this->foreachArtwork(ranges, [](ArtworkItem const &artwork, size_t) {
            artwork->setIsSelected(true); });
        this->updateItems(ranges, QVector<int>() << IsSelectedRole);
    }

    void ArtworksListModel::unselectAllItems() {
        LOG_DEBUG << "#";
        foreachArtwork(Helpers::IndicesRanges(getArtworksSize()), [](ArtworkItem const &artwork, size_t){
            artwork->resetSelected();
        });
        unsetCurrentIndex();
    }

    void ArtworksListModel::updateItems(const Helpers::IndicesRanges &ranges, const QVector<int> &roles) {
        for (auto &r: ranges.getRanges()) {
            QModelIndex topLeft = index(r.first);
            QModelIndex bottomRight = index(r.second);
            emit dataChanged(topLeft, bottomRight, roles);
        }
    }

    void ArtworksListModel::updateSelection(ArtworksListModel::SelectionType selectionType, const QVector<int> &roles) {
        std::function<bool (ArtworkItem const &)> pred;
        switch (selectionType) {
        case SelectionType::All:
            this->updateItems(Helpers::IndicesRanges(getArtworksSize()), roles);
            return;
        case SelectionType::Modified:
            pred = [](ArtworkItem const &artwork) { return artwork->isModified(); };
            break;
        case SelectionType::Selected:
            pred = [](ArtworkItem const &artwork) { return artwork->isSelected(); };
            break;
        default:
            return;
        }

        auto indices = filterArtworks<int>(pred, [](ArtworkItem const &, size_t i) { return (int)i; });
        this->updateItems(Helpers::IndicesRanges(indices), roles);
    }

    std::unique_ptr<Artworks::SessionSnapshot> ArtworksListModel::createSessionSnapshot() {
        auto sessionSnapshot = std::make_unique<Artworks::SessionSnapshot>(
                                   filterAvailableArtworks<ArtworkItem>(
                                       [](ArtworkItem const &artwork, size_t) {return artwork;}),
                m_ArtworksRepository.retrieveFullDirectories());
        return sessionSnapshot;
    }

    Artworks::ArtworksSnapshot ArtworksListModel::createArtworksSnapshot() {
        auto lockers = filterArtworks<ArtworkItem>([](ArtworkItem const &) { return true; },
                [](ArtworkItem const &artwork, size_t) { return artwork; });
        return Artworks::ArtworksSnapshot(lockers);
    }

    void ArtworksListModel::generateAboutToBeRemoved() {
        LOG_DEBUG << "#";
        foreachArtwork([](ArtworkItem const &artwork) { return artwork->isUnavailable(); },
        [](ArtworkItem const &artwork, size_t) {
            Artworks::BasicKeywordsModel &keywordsModel = artwork->getBasicModel();
            keywordsModel.notifyAboutToBeRemoved();
        });
    }

    void ArtworksListModel::unlockAllForIO() {
        LOG_DEBUG << "#";
        foreachArtwork([](ArtworkItem const &){return true;},
        [](ArtworkItem const &artwork, size_t) { artwork->setIsLockedIO(false); });
    }

    bool ArtworksListModel::isInSelectedDirectory(int artworkIndex) {
        bool result = false;
        ArtworkItem artwork;
        if (tryGetArtwork(artworkIndex, artwork)) {
            auto dirID = artwork->getDirectoryID();
            result = m_ArtworksRepository.isDirectorySelected(dirID);
        }
        return result;
    }

    void ArtworksListModel::setCurrentIndex(size_t index) {
        LOG_DEBUG << index;
        if (m_CurrentItemIndex != index) {
            m_CurrentItemIndex = index;

            ArtworkItem artwork;
            if (tryGetArtwork(index, artwork)) {
                using namespace Commands;
                auto editable = std::make_shared<CurrentEditableArtwork>(
                                    artwork,
                                    std::make_shared<ArtworksUpdateTemplate>(*this, getStandardUpdateRoles()));
                sendMessage(editable);
            }
        }
    }

    void ArtworksListModel::unsetCurrentIndex() {
        LOG_DEBUG << "#";
        m_CurrentItemIndex = -1;
        sendMessage(std::shared_ptr<ICurrentEditable>());
    }

    void ArtworksListModel::setItemsSaved(const Helpers::IndicesRanges &ranges) {
        LOG_DEBUG << "#";
        foreachArtwork(ranges, [](ArtworkItem const &artwork, size_t) {
            artwork->resetModified();
        });

        this->updateItems(ranges, QVector<int>() << IsModifiedRole);

        emit modifiedArtworksCountChanged();
        emit artworksChanged(false);
    }

    void ArtworksListModel::detachVectorsFromArtworks(const Helpers::IndicesRanges &ranges) {
        foreachArtworkAs<Artworks::ImageArtwork>(ranges,
                                       [](std::shared_ptr<Artworks::ImageArtwork> const &image) {
            return image->hasVectorAttached(); },
        [this](std::shared_ptr<Artworks::ImageArtwork> const &image, size_t) {
            this->m_ArtworksRepository.removeVector(
                        image->getAttachedVectorPath());
            image->detachVector();
        });

        updateItems(ranges, QVector<int>() << HasVectorAttachedRole);
    }

    void ArtworksListModel::purgeUnavailableFiles() {
        LOG_DEBUG << "#";
        generateAboutToBeRemoved();
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        sendMessage(UnavailableFilesMessage(1));
        deleteUnavailableItems();

        if (m_ArtworksRepository.canPurgeUnavailableFiles()) {
            m_ArtworksRepository.purgeUnavailableFiles();
        } else {
            LOG_INFO << "Unavailable files purging postponed";
        }
    }

    void ArtworksListModel::processUpdateRequests(const std::vector<std::shared_ptr<Services::ArtworkUpdateRequest> > &updateRequests) {
        LOG_INFO << updateRequests.size() << "requests to process";
        if (m_ArtworkList.empty()) { return; }

        std::vector<int> indicesToUpdate;
        indicesToUpdate.reserve(updateRequests.size());
        QSet<int> rolesToUpdateSet;
        int cacheMisses = 0;

        for (auto &request: updateRequests) {
            size_t index = request->getLastKnownIndex();
            if (index >= m_ArtworkList.size()) { continue; }
            ArtworkItem artwork;
            if (tryGetArtwork(index, artwork) &&
                    (artwork->getItemID() == request->getArtworkID())) {
                indicesToUpdate.push_back((int)index);
                rolesToUpdateSet.unite(request->getRolesToUpdate());
            } else {
                LOG_VERBOSE << "Cache miss. Found" << (artwork ? artwork->getItemID() : -1) << "instead of" << request->getArtworkID();
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
        if (m_ArtworkList.empty()) { return; }

        std::vector<int> indices = filterArtworks<int>(
                    [&artworkIDs](ArtworkItem const &artwork) { return artworkIDs.contains(artwork->getItemID().get()); },
                [](ArtworkItem const &, size_t index) { return (int)index; });

        this->updateItems(Helpers::IndicesRanges(indices), rolesToUpdate);
    }

    ArtworksAddResult ArtworksListModel::addFiles(const std::shared_ptr<Filesystem::IFilesCollection> &filesCollection,
                                                  Common::AddFilesFlags flags) {
        const int count = (int)getArtworksSize();
        QSet<qint64> fullDirectories;
        Artworks::ArtworksSnapshot snapshot = addArtworks(filesCollection, fullDirectories);
        const bool autoAttach = Common::HasFlag(flags, Common::AddFilesFlags::FlagAutoFindVectors);
        const int attachedCount = attachVectors(filesCollection, snapshot, count, autoAttach);
        m_ArtworksRepository.watchFiles(snapshot);
        m_ArtworksRepository.setFullDirectories(fullDirectories);

        return ArtworksAddResult(snapshot, attachedCount);
    }

    Artworks::ArtworksSnapshot ArtworksListModel::addArtworks(std::shared_ptr<Filesystem::IFilesCollection> const &filesCollection,
                                                              QSet<qint64> &fullDirectories) {
        const int newFilesCount = m_ArtworksRepository.getNewFilesCount(filesCollection);
        Artworks::ArtworksSnapshot snapshot;
        if (newFilesCount == 0) { return snapshot; }

        snapshot.reserve(newFilesCount);
        const int count = (int)getArtworksSize();

        emit beginInsertRows(QModelIndex(), count, count + newFilesCount - 1);
        {
            for (auto &file: filesCollection->getFiles()) {
                if (file.m_Type == Filesystem::ArtworkFileType::Vector) { continue; }
                qint64 directoryID = 0;
                auto flags = m_ArtworksRepository.accountFile(file.m_Path, directoryID);
                if (flags != Common::AccountFileFlags::None) {
                    ArtworkItem artwork = createArtwork(file, directoryID);
                    m_ArtworkList.push_back(artwork);
                    snapshot.append(artwork);
                    connectArtworkSignals(artwork.get());
                    if (file.isPartOfFullDirectory()) { fullDirectories.insert(directoryID); }
                }
            }
        }
        emit endInsertRows();

        syncArtworksIndices(count, -1);

        return snapshot;
    }

    ArtworksRemoveResult ArtworksListModel::removeFiles(Helpers::IndicesRanges const &ranges) {
        int selectedCount = 0;
        bool anyToDelete = false;
        foreachArtwork(ranges, [this, &selectedCount, &anyToDelete](ArtworkItem const &artwork, size_t) {
            artwork->setRemoved();
            anyToDelete = true;
            if (artwork->isSelected()) {
                artwork->resetSelected();
                selectedCount++;
            }
        });

        if (!anyToDelete) { return ArtworksRemoveResult(); }

        if (selectedCount > 0) { emit selectedArtworksRemoved(selectedCount); }
        emit modifiedArtworksCountChanged();
        emit artworksChanged(true);

        Artworks::ArtworksSnapshot snapshot(
                    filterArtworks<ArtworkItem>(
                        [](ArtworkItem const &artwork) { return artwork->isRemoved(); },
                    [](ArtworkItem const &artwork, size_t) { return artwork; }));

        ArtworksRemoveResult removeResult;
        m_ArtworksRepository.removeFiles(snapshot, removeResult);
        removeResult.m_RemovedCount = snapshot.size();
        removeResult.m_RemovedRanges = ranges;
        return removeResult;
    }

    ArtworksRemoveResult ArtworksListModel::removeFilesFromDirectory(int directoryIndex) {
        LOG_INFO << "Remove artworks directory at" << directoryIndex;
        const QString &directory = m_ArtworksRepository.getDirectoryPath(directoryIndex);
        LOG_FOR_TESTS << "Removing directory:" << directory;

        const QString directoryAbsolutePath = QDir(directory).absolutePath();
        std::vector<int> indices = filterArtworks<int>(
                    [&directoryAbsolutePath](ArtworkItem const &artwork) {
                // if we will remove all files in directory then undo operation might
                // restore files that were removed separately from same directory
                return artwork->isInDirectory(directoryAbsolutePath) && !artwork->isRemoved(); },
                [](ArtworkItem const &, size_t index) { return (int)index; });

        return removeFiles(Helpers::IndicesRanges(indices));
    }

    void ArtworksListModel::restoreRemoved(Helpers::IndicesRanges const &ranges) {
        LOG_DEBUG << "#";
        int restoredCount = foreachArtwork(ranges,
                                [](ArtworkItem const &artwork) { return artwork->isRemoved(); },
                [this](ArtworkItem const &artwork, size_t) {
            qint64 directoryID;
            auto flags = this->m_ArtworksRepository.accountFile(artwork->getFilepath(), directoryID);
            Q_ASSERT(flags == Common::AccountFileFlags::FlagRepositoryModified);
            if (flags != Common::AccountFileFlags::None) {
                artwork->resetRemoved();
            }
        });

        LOG_INFO << restoredCount << "artworks restored";

        updateSelection(SelectionType::All, QVector<int>() << IsSelectedRole);
        emit artworksChanged(true);
        // TODO: finish this
    }

    void ArtworksListModel::deleteRemovedItems() {
        LOG_DEBUG << "#";
        auto indices = filterArtworks<int>(
                    [](ArtworkItem const &artwork){ return artwork->isRemoved(); },
                [](ArtworkItem const &, size_t index) { return (int)index; });
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

        for (auto &artwork: artworksToDestroy) {
            destroyArtwork(artwork);
        }
    }

    void ArtworksListModel::deleteItems(const Helpers::IndicesRanges &ranges) {
        LOG_DEBUG << ranges.size() << "range(s)";
        QModelIndex dummy;
        const bool willReset = ranges.length() > 20;
        if (willReset) { emit beginResetModel(); }
        auto &rangesArray = ranges.getRanges();
        const int size = (int)rangesArray.size();

        for (int i = size - 1; i >= 0; i--) {
            auto &r = rangesArray.at(i);
            Q_ASSERT(r.first >= 0 && (size_t)r.first < getArtworksSize());
            Q_ASSERT(r.second >= 0 && (size_t)r.second < getArtworksSize());

            auto itBegin = m_ArtworkList.begin() + r.first;
            auto itEnd = m_ArtworkList.begin() + (r.second + 1);

            std::vector<ArtworkItem> itemsToDelete(itBegin, itEnd);
            if (!willReset) { emit beginRemoveRows(dummy, r.first, r.second); }
            {
                m_ArtworkList.erase(itBegin, itEnd);
            }
            if (!willReset) { emit endRemoveRows(); }

            std::vector<ArtworkItem>::iterator it = itemsToDelete.begin();
            std::vector<ArtworkItem>::iterator itemsEnd = itemsToDelete.end();
            for (; it < itemsEnd; it++) {
                auto &artwork = *it;
                Q_ASSERT(artwork->isRemoved());
                LOG_VERBOSE << "File removed:" << artwork->getFilepath();
                destroyArtwork(artwork);
            }
        }

        if (willReset) { emit endResetModel(); }
        syncArtworksIndices();
    }

    void ArtworksListModel::deleteUnavailableItems() {
        LOG_DEBUG << "#";
        auto indices = filterArtworks<int>(
                    [](ArtworkItem const &artwork){ return artwork->isUnavailable(); },
                [](ArtworkItem const &, size_t index) { return (int)index; });
        Helpers::IndicesRanges ranges(indices);
        removeFiles(ranges);
        deleteItems(ranges);
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

            vectors[absolutePath].insert(fi.baseName().toLower(), file.m_Path);
        }

        QVector<int> indicesToUpdate;
        int attachedCount = attachKnownVectors(vectors, indicesToUpdate);

        if (autoAttach) {
            QVector<int> autoAttachedIndices;
            attachedCount += Helpers::findAndAttachVectors(snapshot, autoAttachedIndices);

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

        const size_t size = getArtworksSize();
        indicesToUpdate.reserve((int)size);

        for (size_t i = 0; i < size; ++i) {
            auto &artwork = accessArtwork(i);
            auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
            if (image == nullptr) { continue; }

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
                         this, &ArtworksListModel::modifiedArtworksCountChanged);

        QObject::connect(artwork, &Artworks::ArtworkMetadata::editingPaused,
                         this, &ArtworksListModel::onArtworkEditingPaused);

        QObject::connect(artwork, &Artworks::ArtworkMetadata::selectedChanged,
                         this, &ArtworksListModel::artworkSelectedChanged);
    }

    void ArtworksListModel::syncArtworksIndices(int startIndex, int count) {
        if (count == -1) { count = getArtworksSize(); }
        foreachArtwork(Helpers::IndicesRanges(startIndex, count),
        [this](ArtworkItem const &, size_t i) { this->accessArtwork(i); });
    }

    void ArtworksListModel::resetSpellCheckResults() {
        foreachArtwork(Helpers::IndicesRanges(getArtworksSize()),
                       [](ArtworkItem const &artwork, size_t) {
            artwork->resetSpellingInfo();
        });
    }

    void ArtworksListModel::resetDuplicatesResults() {
        foreachArtwork(Helpers::IndicesRanges(getArtworksSize()),
                       [](ArtworkItem const &artwork, size_t) {
            artwork->resetDuplicatesInfo();
        });
    }

    void ArtworksListModel::spellCheckAll() {
        LOG_DEBUG << "#";
        std::vector<std::shared_ptr<Artworks::IBasicModelSource>> artworks(m_ArtworkList.begin(), m_ArtworkList.end());
        sendMessage(artworks);
    }

    QVariant ArtworksListModel::data(const QModelIndex &index, int role) const {
        int row = index.row();

        if (row < 0 || (size_t)row >= getArtworksSize()) {
            return QVariant();
        }

        auto &artwork = accessArtwork(row);
        switch (role) {
            case ArtworkDescriptionRole:
                return artwork->getDescription();
            case ArtworkFilenameRole:
                return artwork->getFilepath();
            case ArtworkTitleRole:
                return artwork->getTitle();
            case KeywordsStringRole: {
                auto &keywordsModel = artwork->getBasicModel();
                return keywordsModel.getKeywordsString();
            }
            case IsModifiedRole:
                return artwork->isModified();
            case IsSelectedRole:
                return artwork->isSelected();
            case KeywordsCountRole: {
                auto &keywordsModel = artwork->getBasicModel();
                return keywordsModel.getKeywordsCount();
            }
            case HasVectorAttachedRole: {
                auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
                return (image != nullptr) && image->hasVectorAttached();
            }
            case BaseFilenameRole:
                return artwork->getBaseFilename();
            case IsVideoRole: {
                bool isVideo = std::dynamic_pointer_cast<Artworks::VideoArtwork>(artwork) != nullptr;
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

        if (row < 0 || (size_t)row >= getArtworksSize()) {
            return Qt::ItemIsEnabled;
        }

        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    bool ArtworksListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
        int row = index.row();

        if (row < 0 || (size_t)row >= getArtworksSize()) {
            return false;
        }

        ArtworkItem const &metadata = accessArtwork(row);
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

    Artworks::ArtworkMetadata *ArtworksListModel::getArtworkObject(int index) const {
        Artworks::ArtworkMetadata *item = NULL;

        if (0 <= index && (size_t)index < getArtworksSize()) {
            item = accessArtwork(index).get();
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        }

        return item;
    }

    Artworks::BasicMetadataModel *ArtworksListModel::getBasicModelObject(int index) const {
        Artworks::BasicMetadataModel *keywordsModel = NULL;

        if (0 <= index && (size_t)index < getArtworksSize()) {
            keywordsModel = &accessArtwork(index)->getBasicMetadataModel();
            QQmlEngine::setObjectOwnership(keywordsModel, QQmlEngine::CppOwnership);
        }

        return keywordsModel;
    }

    bool ArtworksListModel::tryGetArtwork(size_t index, ArtworkItem &item) const {
        bool found = false;
        if (index < m_ArtworkList.size()) {
            item = accessArtwork(index);
            found = true;
        }
        return found;
    }

    std::shared_ptr<Commands::ICommand> ArtworksListModel::removeKeywordAt(int artworkIndex, int keywordIndex) {
        LOG_INFO << "artwork index" << artworkIndex << "| keyword index" << keywordIndex;
        std::shared_ptr<Commands::ICommand> command;        
        ArtworkItem artwork;
        if (tryGetArtwork(artworkIndex, artwork)) {
            setCurrentIndex(artworkIndex);
            using namespace Commands;
            command = std::make_shared<ArtworksCommand>(
                          Artworks::ArtworksSnapshot({artwork}),
                          std::make_shared<ArtworksTemplateComposite>(
                              std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                  std::make_shared<KeywordEditTemplate>(
                                  Common::KeywordEditFlags::Remove,
                                  keywordIndex),
                                  std::make_shared<ArtworksUpdateTemplate>(*this,
                                  QVector<int>() << IsModifiedRole << KeywordsCountRole)
                              }));
        }

        return command;
    }

    std::shared_ptr<Commands::ICommand> ArtworksListModel::removeLastKeyword(int artworkIndex) {
        LOG_INFO << "index" << artworkIndex;
        std::shared_ptr<Commands::ICommand> command;
        auto &artwork = accessArtwork(artworkIndex);
        if (artwork != nullptr) {
            setCurrentIndex(artworkIndex);
            using namespace Commands;
            command = std::make_shared<ArtworksCommand>(
                          Artworks::ArtworksSnapshot({artwork}),
                          std::make_shared<ArtworksTemplateComposite>(
                              std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                  std::make_shared<KeywordEditTemplate>(
                                  Common::KeywordEditFlags::RemoveLast),
                                  std::make_shared<ArtworksUpdateTemplate>(*this,
                                  QVector<int>() << IsModifiedRole << KeywordsCountRole)
                              }));
        } else {
            command = std::make_shared<Commands::EmptyCommand>();
        }
        return command;
    }

    std::shared_ptr<Commands::ICommand> ArtworksListModel::appendKeyword(int artworkIndex, const QString &keyword) {
        LOG_INFO << "artwork index" << artworkIndex << "| keyword" << keyword;
        std::shared_ptr<Commands::ICommand> command;
        ArtworkItem artwork;
        if (tryGetArtwork(artworkIndex, artwork)) {
            setCurrentIndex(artworkIndex);
            using namespace Commands;
            command = std::make_shared<ArtworksCommand>(
                          Artworks::ArtworksSnapshot({artwork}),
                          std::make_shared<ArtworksTemplateComposite>(
                              std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                  std::make_shared<EditArtworksTemplate>("", "",
                                  QStringList() << keyword,
                                  Common::ArtworkEditFlags::AppendKeywords |
                                  Common::ArtworkEditFlags::EditKeywords),
                                  std::make_shared<ArtworksUpdateTemplate>(*this,
                                  QVector<int>() << IsModifiedRole << KeywordsCountRole)
                              }));
        } else {
            command = std::make_shared<Commands::EmptyCommand>();
        }
        return command;
    }

    std::shared_ptr<Commands::ICommand> ArtworksListModel::pasteKeywords(int artworkIndex, const QStringList &keywords) {
        std::shared_ptr<Commands::ICommand> command;
        LOG_INFO << "artwork index" << artworkIndex << "|" << keywords;
        ArtworkItem artwork;
        if (tryGetArtwork(artworkIndex, artwork) && !keywords.empty()) {
            setCurrentIndex(artworkIndex);

            if (keywords.length() == 1) {
                // TODO: add test for this
                LOG_INFO << "Pasting only one keyword. Leaving it in the edit box.";
                return std::make_shared<Commands::EmptyCommand>();
            }

            using namespace Commands;
            command = std::make_shared<ModifyArtworksCommand>(
                          artwork,
                          std::make_shared<ArtworksTemplateComposite>(
                              std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                  std::make_shared<EditArtworksTemplate>("", "",
                                  keywords,
                                  Common::ArtworkEditFlags::AppendKeywords |
                                  Common::ArtworkEditFlags::EditKeywords),
                                  std::make_shared<ArtworksUpdateTemplate>(
                                  *this, getStandardUpdateRoles())}));
        } else {
            command = std::make_shared<Commands::EmptyCommand>();
        }
        return command;
    }

    std::shared_ptr<Commands::ICommand> ArtworksListModel::addSuggestedKeywords(int artworkIndex, const QStringList &keywords) {
        std::shared_ptr<Commands::ICommand> command;
        LOG_DEBUG << "artwork index" << artworkIndex;
        ArtworkItem artwork;
        if (tryGetArtwork(artworkIndex, artwork) && !keywords.empty()) {
            setCurrentIndex(artworkIndex);
            using namespace Commands;
            command = std::make_shared<ModifyArtworksCommand>(
                          artwork,
                          std::make_shared<ArtworksTemplateComposite>(
                              std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                  std::make_shared<EditArtworksTemplate>("", "",
                                  keywords,
                                  Common::ArtworkEditFlags::AppendKeywords |
                                  Common::ArtworkEditFlags::EditKeywords),
                                  std::make_shared<ArtworksUpdateTemplate>(
                                  *this, getStandardUpdateRoles())}));
        } else {
            command = std::make_shared<Commands::EmptyCommand>();
        }
        return command;
    }

    std::shared_ptr<Commands::ICommand> ArtworksListModel::editKeyword(int artworkIndex, int keywordIndex, const QString &replacement) {
        LOG_INFO << "metadata index:" << artworkIndex;
        std::shared_ptr<Commands::ICommand> command;
        auto &artwork = accessArtwork(artworkIndex);
        if (artwork != nullptr) {
            setCurrentIndex(artworkIndex);
            using namespace Commands;
            command = std::make_shared<ArtworksCommand>(
                          Artworks::ArtworksSnapshot({artwork}),
                          std::make_shared<ArtworksTemplateComposite>(
                              std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                  std::make_shared<KeywordEditTemplate>(
                                  Common::KeywordEditFlags::Replace,
                                  keywordIndex,
                                  replacement),
                                  std::make_shared<ArtworksUpdateTemplate>(*this,
                                  QVector<int>() << IsModifiedRole << KeywordsCountRole)
                              }));
        } else {
            command = std::make_shared<Commands::EmptyCommand>();
        }
        return command;
    }

    std::shared_ptr<Commands::ICommand> ArtworksListModel::plainTextEdit(int artworkIndex, const QString &rawKeywords, bool spaceIsSeparator) {
        LOG_DEBUG << "Plain text edit for item" << artworkIndex;
        std::shared_ptr<Commands::ICommand> command;
        ArtworkItem artwork;
        if (tryGetArtwork(artworkIndex, artwork)) {
            setCurrentIndex(artworkIndex);
            QVector<QChar> separators;
            separators << QChar(',');
            if (spaceIsSeparator) { separators << QChar::Space; }
            QStringList keywords;
            Helpers::splitKeywords(rawKeywords.trimmed(), separators, keywords);

            using namespace Commands;
            command = std::make_shared<ModifyArtworksCommand>(
                          artwork,
                          std::make_shared<ArtworksTemplateComposite>(
                              std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                  std::make_shared<EditArtworksTemplate>("", "",
                                  keywords,
                                  Common::ArtworkEditFlags::EditKeywords),
                                  std::make_shared<ArtworksUpdateTemplate>(
                                  *this, getStandardUpdateRoles())}));
        } else {
            command = std::make_shared<Commands::EmptyCommand>();
        }
        return command;
    }

    std::shared_ptr<Commands::ICommand> ArtworksListModel::expandPreset(int artworkIndex,
                                                                        int keywordIndex,
                                                                        unsigned int presetID,
                                                                        KeywordsPresets::IPresetsManager &presetsManager) {
        LOG_INFO << "item" << artworkIndex << "keyword" << keywordIndex << "preset" << presetID;
        std::shared_ptr<Commands::ICommand> command;
        ArtworkItem artwork;
        if (tryGetArtwork(artworkIndex, artwork)) {
            setCurrentIndex(artworkIndex);
            using namespace Commands;
            command = std::make_shared<ModifyArtworksCommand>(
                          artwork,
                          std::make_shared<ArtworksTemplateComposite>(
                              std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                  std::make_shared<ExpandPresetTemplate>(presetsManager,
                                  (KeywordsPresets::ID_t)presetID,
                                  keywordIndex),
                                  std::make_shared<ArtworksUpdateTemplate>(
                                  *this, getStandardUpdateRoles())}));
        } else {
            command = std::make_shared<Commands::EmptyCommand>();
        }
        return command;
    }

    std::shared_ptr<Commands::ICommand> ArtworksListModel::expandLastAsPreset(int artworkIndex,
                                                                              KeywordsPresets::IPresetsManager &presetsManager) {
        LOG_INFO << "item" << artworkIndex;
        std::shared_ptr<Commands::ICommand> command = std::make_shared<Commands::EmptyCommand>();
        ArtworkItem artwork;
        if (tryGetArtwork(artworkIndex, artwork)) {
            setCurrentIndex(artworkIndex);

            auto &basicModel = artwork->getBasicModel();
            int keywordIndex = basicModel.getKeywordsCount() - 1;
            QString lastKeyword = basicModel.retrieveKeyword(keywordIndex);

            KeywordsPresets::ID_t presetID;
            if (presetsManager.tryFindSinglePresetByName(lastKeyword, false, presetID)) {
                using namespace Commands;
                command = std::make_shared<ModifyArtworksCommand>(
                              artwork,
                              std::make_shared<ArtworksTemplateComposite>(
                                  std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                      std::make_shared<ExpandPresetTemplate>(presetsManager,
                                      presetID,
                                      keywordIndex),
                                      std::make_shared<ArtworksUpdateTemplate>(
                                      *this, getStandardUpdateRoles())}));
            }
        }
        return command;
    }

    std::shared_ptr<Commands::ICommand> ArtworksListModel::addPreset(int artworkIndex, unsigned int presetID, KeywordsPresets::IPresetsManager &presetsManager) {
        LOG_INFO << "item" << artworkIndex << "preset" << presetID;
        std::shared_ptr<Commands::ICommand> command;
        ArtworkItem artwork;
        if (tryGetArtwork(artworkIndex, artwork)) {
            setCurrentIndex(artworkIndex);
            using namespace Commands;
            command = std::make_shared<ModifyArtworksCommand>(
                          artwork,
                          std::make_shared<ArtworksTemplateComposite>(
                              std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                  std::make_shared<ExpandPresetTemplate>(presetsManager,
                                  (KeywordsPresets::ID_t)presetID),
                                  std::make_shared<ArtworksUpdateTemplate>(
                                  *this, getStandardUpdateRoles())}));
        } else {
            command = std::make_shared<Commands::EmptyCommand>();
        }
        return command;
    }

    std::shared_ptr<Commands::ICommand> ArtworksListModel::acceptCompletionAsPreset(int artworkIndex,
                                                                                    int completionID,
                                                                                    KeywordsPresets::IPresetsManager &presetsManager,
                                                                                    AutoComplete::ICompletionSource &completionsSource) {
        LOG_INFO << "item" << artworkIndex << "completionID" << completionID;
        std::shared_ptr<Commands::ICommand> command = std::make_shared<Commands::EmptyCommand>();
        ArtworkItem artwork;
        if (tryGetArtwork(artworkIndex, artwork)) {
            setCurrentIndex(artworkIndex);

            auto completionItem = completionsSource.getAcceptedCompletion(completionID);
            if (!completionItem) {
                LOG_WARNING << "Completion is not available anymore";
                return command;
            }

            if (completionItem->isPreset() ||
                    (completionItem->canBePreset() && completionItem->shouldExpandPreset())) {
                const int presetID = completionItem->getPresetID();
                using namespace Commands;
                command = std::make_shared<ModifyArtworksCommand>(
                            artwork,
                            std::make_shared<ArtworksTemplateComposite>(
                                std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                    std::make_shared<ExpandCompletionPreset>(completionID,
                                    presetsManager,
                                    (KeywordsPresets::ID_t)presetID),
                                    std::make_shared<ArtworksUpdateTemplate>(
                                    *this, getStandardUpdateRoles())}));
            }
            /* --------- this is handled in the edit field -----------
                else if (completionItem->isKeyword()) {
                this->appendKeyword(metadataIndex, completionItem->getCompletion());
                accepted = true;
            }*/
        }

        return command;
    }

    std::shared_ptr<Commands::ICommand> ArtworksListModel::removeMetadata(const Helpers::IndicesRanges &ranges,
                                                                          Common::ArtworkEditFlags flags) {
        auto weakSnapshot = filterArtworks<ArtworkItem>(
                                ranges,
                                [](ArtworkItem const &) { return true; },
                [](ArtworkItem const &artwork, size_t) { return artwork; });
        using namespace Commands;
        return std::make_shared<ModifyArtworksCommand>(
                    Artworks::ArtworksSnapshot(weakSnapshot),
                    std::make_shared<ArtworksTemplateComposite>(
                        std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                            std::make_shared<EditArtworksTemplate>(flags | Common::ArtworkEditFlags::Clear),
                            std::make_shared<ArtworksUpdateTemplate>(
                            *this, getStandardUpdateRoles())}));
    }

    void ArtworksListModel::onFilesUnavailableHandler() {
        LOG_DEBUG << "#";
        bool anyArtworkUnavailable = false;
        bool anyVectorUnavailable = false;

        foreachArtwork([this](ArtworkItem const &artwork) {
            return this->m_ArtworksRepository.isFileUnavailable(artwork->getFilepath()); },
        [&anyArtworkUnavailable](ArtworkItem const &artwork, size_t) {
            artwork->setUnavailable(); anyArtworkUnavailable = true; });

        foreachArtworkAs<Artworks::ImageArtwork>(Helpers::IndicesRanges(getArtworksSize()),
                    [this](std::shared_ptr<Artworks::ImageArtwork> const &image) { return image->hasVectorAttached() &&
                    this->m_ArtworksRepository.isFileUnavailable(image->getAttachedVectorPath()); },
        [&anyVectorUnavailable](std::shared_ptr<Artworks::ImageArtwork> const &image, size_t) {
            image->detachVector(); anyVectorUnavailable = true; });

        if (anyArtworkUnavailable) {
            emit unavailableArtworksFound();
        } else if (anyVectorUnavailable) {
            emit unavailableVectorsFound();
        }
    }

    void ArtworksListModel::onArtworkEditingPaused() {
        LOG_DEBUG << "#";
        Artworks::ArtworkMetadata *artwork = qobject_cast<Artworks::ArtworkMetadata *>(sender());
        Q_ASSERT(artwork != nullptr);
        if (artwork != nullptr) {
            sendMessage(artwork->getptr());
        }
    }

    void ArtworksListModel::onUndoStackEmpty() {
        LOG_DEBUG << "#";
        deleteRemovedItems();
    }

    void ArtworksListModel::onSpellCheckerAvailable() {
        spellCheckAll();
    }

    void ArtworksListModel::onSpellCheckDisabled() {
        resetSpellCheckResults();
    }

    void ArtworksListModel::onDuplicatesDisabled() {
        resetDuplicatesResults();
    }

    void ArtworksListModel::userDictUpdateHandler(const QStringList &keywords, bool overwritten) {
        LOG_DEBUG << "#";
        for (auto &artwork: m_ArtworkList) {
            auto &basicModel = artwork->getBasicMetadataModel();
            SpellCheck::SpellCheckInfo &info = basicModel.getSpellCheckInfo();
            if (!overwritten) {
                info.removeWordsFromErrors(keywords);
            } else {
                info.clear();
            }
        }

        spellCheckAll();
    }

    void ArtworksListModel::userDictClearedHandler() {
        LOG_DEBUG << "#";
        spellCheckAll();
    }

    void ArtworksListModel::onMetadataWritingFinished() {
        LOG_DEBUG << "#";
        unlockAllForIO();
        updateSelection(ArtworksListModel::SelectionType::Selected, QVector<int>() << IsModifiedRole);
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

    ArtworksListModel::ArtworkItem ArtworksListModel::createArtwork(const Filesystem::ArtworkFile &file, qint64 directoryID) {
        ArtworkItem artwork;
        if (file.m_Type == Filesystem::ArtworkFileType::Image) {
            artwork.reset(new Artworks::ImageArtwork(file.m_Path, getNextID(), directoryID),
                          artworkDeleter);
        } else if (file.m_Type == Filesystem::ArtworkFileType::Video) {
            artwork.reset(new Artworks::VideoArtwork(file.m_Path, getNextID(), directoryID),
                          artworkDeleter);
        }
        Q_ASSERT(artwork != nullptr);
        return artwork;
    }

    std::shared_ptr<Artworks::ArtworkMetadata> const &ArtworksListModel::accessArtwork(size_t index) const {
        Q_ASSERT(index < m_ArtworkList.size());
        auto &artwork = m_ArtworkList.at(index);
        artwork->setCurrentIndex(index);
        return artwork;
    }

    void ArtworksListModel::destroyArtwork(ArtworkItem const &artwork) {
        LOG_VERBOSE << "Destroying metadata" << artwork->getItemID();

        bool disconnectStatus = QObject::disconnect(artwork.get(), 0, this, 0);
        if (disconnectStatus == false) { LOG_DEBUG << "Disconnect Artwork from ArtworksListModel returned false"; }
        disconnectStatus = QObject::disconnect(this, 0, artwork.get(), 0);
        if (disconnectStatus == false) { LOG_DEBUG << "Disconnect ArtworksListModel from Artwork returned false"; }

#if !defined(CORE_TESTS) && !defined(INTEGRATION_TESTS)
        artwork->getBasicMetadataModel().clearModel();
#endif
        artwork->deepDisconnect();
        artwork->clearSpellingInfo();
    }

    int ArtworksListModel::getNextID() {
        return m_LastID++;
    }

    int ArtworksListModel::foreachArtwork(const Helpers::IndicesRanges &ranges,
                                          std::function<bool (ArtworkItem const &)> pred,
                                          std::function<void (ArtworkItem const &, size_t)> action) const {
        int itemsProcessed = 0;
        for (auto &r: ranges.getRanges()) {
            if ((r.first < 0) || (r.second >= m_ArtworkList.size())) { continue; }

            for (int i = r.first; i <= r.second; i++) {
                auto &artwork = accessArtwork(i);
                if (pred(artwork)) {
                    action(artwork, i);
                    itemsProcessed++;
                }
            }
        }
        return itemsProcessed;
    }

    int ArtworksListModel::foreachArtwork(std::function<bool (ArtworkItem const &)> pred,
                                          std::function<void (ArtworkItem const &, size_t)> action) const {
        return foreachArtwork(Helpers::IndicesRanges(0, (int)getArtworksSize()),
                              pred,
                              action);
    }

    int ArtworksListModel::foreachArtwork(const Helpers::IndicesRanges &ranges,
                                           std::function<void (ArtworkItem const &, size_t)> action) const {
        return foreachArtwork(ranges,
                              [](ArtworkItem const &){ return true; },
        action);
    }
}
