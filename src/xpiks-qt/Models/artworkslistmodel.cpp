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
#include "artworksrepository.h"
#include "../Helpers/artworkshelpers.h"

namespace Models {
    ArtworksListModel::ArtworksListModel(ArtworksRepository &repository, QObject *parent):
        QAbstractListModel(parent),
        m_ArtworksRepository(repository),
        // all items before 1024 are reserved for internal models
        m_LastID(1024)
    {
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
        size_t size = m_ArtworkList.size();

        for (size_t i = 0; i < size; ++i) {
            if (accessArtwork(i)->isModified()) {
                modifiedCount++;
            }
        }

        return modifiedCount;
    }

    void ArtworksListModel::updateItems(const Helpers::IndicesRanges &ranges, const QVector<int> &roles) {
        for (auto &r: ranges.getRanges()) {
            QModelIndex topLeft = index(r.first);
            QModelIndex bottomRight = index(r.second);
            emit dataChanged(topLeft, bottomRight, roles);
        }
    }

    std::unique_ptr<MetadataIO::SessionSnapshot> ArtworksListModel::snapshotAll() {
        std::unique_ptr<MetadataIO::SessionSnapshot> sessionSnapshot(
                    new MetadataIO::SessionSnapshot(
                        selectAvailableArtworks([](ArtworkMetadata *artwork, size_t) {return artwork;}),
                        m_ArtworksRepository.retrieveFullDirectories()));
        return sessionSnapshot;
    }

    ArtworksListModel::ArtworksAddResult ArtworksListModel::addFiles(const std::shared_ptr<Filesystem::IFilesCollection> &filesCollection,
                                                                     Common::AddFilesFlags flags) {
        const int newFilesCount = m_ArtworksRepository.getNewFilesCount(filesCollection);
        MetadataIO::ArtworksSnapshot snapshot;
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
            destroyInnerItem(artwork);
        }
    }

    int ArtworksListModel::attachVectors(const std::shared_ptr<Filesystem::IFilesCollection> &filesCollection,
                                         const MetadataIO::ArtworksSnapshot &snapshot,
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
            ImageArtwork *image = dynamic_cast<ImageArtwork *>(metadata);
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

    void ArtworksListModel::connectArtworkSignals(ArtworkMetadata *artwork) {
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

    int ArtworksListModel::getNextID() {
        return m_LastID++;
    }

    void ArtworksListModel::foreachArtwork(std::function<bool (ArtworkMetadata *)> pred,
                                           std::function<void (ArtworkMetadata *, size_t)> action) const {
        size_t i = 0;
        for (auto *artwork: m_ArtworkList) {
            i++;
            if (pred(artwork)) {
                action(artwork, i);
            }
        }
    }
}
