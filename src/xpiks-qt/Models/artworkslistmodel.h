/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKSLISTMODEL_H
#define ARTWORKSLISTMODEL_H

#include <QAbstractListModel>
#include <functional>
#include <deque>
#include <vector>
#include "../QMLExtensions/artworkupdaterequest.h"
#include "artworkmetadata.h"
#include "../Helpers/ifilenotavailablemodel.h"
#include "../MetadataIO/artworkssnapshot.h"
#include "../Filesystem/ifilescollection.h"
#include "../Helpers/indicesranges.h"

namespace Models {
    class ArtworksRepository;

    class ArtworksListModel:
            public QAbstractListModel,
            public Helpers::IFileNotAvailableModel
    {
        Q_OBJECT
        Q_PROPERTY(int modifiedArtworksCount READ getModifiedArtworksCount NOTIFY modifiedArtworksCountChanged)

#ifndef CORE_TESTS
        typedef std::deque<ArtworkMetadata *> ArtworksContainer;
#else
        typedef std::vector<ArtworkMetadata *> ArtworksContainer;
#endif

    public:
        ArtworksListModel(ArtworksRepository &repository, QObject *parent=0);
        virtual ~ArtworksListModel();

    public:
        enum ArtItemsModel_Roles {
            ArtworkDescriptionRole = Qt::UserRole + 1,
            EditArtworkDescriptionRole,
            ArtworkFilenameRole,
            ArtworkTitleRole,
            EditArtworkTitleRole,
            KeywordsStringRole,
            KeywordsCountRole,
            IsModifiedRole,
            IsSelectedRole,
            EditIsSelectedRole,
            HasVectorAttachedRole,
            BaseFilenameRole,
            IsVideoRole,
            ArtworkThumbnailRole,
            IsReadOnlyRole,
            RolesNumber
        };

        struct ArtworksAddResult {
            MetadataIO::ArtworksSnapshot m_Snapshot;
            int m_AttachedVectorsCount;
        };

        struct ArtworksRemoveResult {
            QSet<qint64> m_SelectedDirectoryIds;
            int m_RemovedCount;
            bool m_UnselectAll;
        };

    public:
        size_t getArtworksCount() const { return m_ArtworkList.size(); }
        int getModifiedArtworksCount();
        QVector<int> getArtworkStandardRoles() const;

    public:
        // general purpose collective methods
        void updateItems(const Helpers::IndicesRanges &ranges, const QVector<int> &roles);
        std::unique_ptr<MetadataIO::SessionSnapshot> snapshotAll();
        void generateAboutToBeRemoved();
        void unlockAllForIO();

    public:
        // update hub related
        void processUpdateRequests(const std::vector<std::shared_ptr<QMLExtensions::ArtworkUpdateRequest> > &updateRequests);
        void updateArtworksByIDs(const QSet<qint64> &artworkIDs, const QVector<int> &rolesToUpdate);

    public:
        // add-remove items methods
        ArtworksAddResult addFiles(const std::shared_ptr<Filesystem::IFilesCollection> &filesCollection,
                                   Common::AddFilesFlags flags);
        ArtworksRemoveResult removeFiles(const Helpers::IndicesRanges &ranges);
        ArtworksRemoveResult removeFilesFromDirectory(int directoryIndex);
        void restoreRemoved();
        void deleteRemovedItems();
        void deleteUnavailableItems();
        void deleteAllItems();

    private:
        // general purpose internal methods
        MetadataIO::ArtworksSnapshot deleteItems(const Helpers::IndicesRanges &ranges);
        int attachVectors(const std::shared_ptr<Filesystem::IFilesCollection> &filesCollection,
                           const MetadataIO::ArtworksSnapshot &snapshot,
                           int initialCount,
                           bool autoAttach);
        int attachKnownVectors(const QHash<QString, QHash<QString, QString> > &vectorsPaths,
                               QVector<int> &indicesToUpdate) const;
        void connectArtworkSignals(ArtworkMetadata *artwork);
        void syncArtworksIndices();

    public:
        // qabstractlistmodel methods
        virtual int rowCount(const QModelIndex &) const { return (int)getArtworksCount(); }
        virtual QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) override;

    signals:
        void modifiedArtworksCountChanged();
        void artworksChanged(bool needToMoveCurrentItem);
        void artworksReimported(int importID, int artworksCount);
        void selectedArtworksRemoved(int count);
        void fileWithIndexUnavailable(size_t index);
        void unavailableArtworksFound();
        void unavailableVectorsFound();
        void userDictUpdate(const QString &word);
        void artworkSelectedChanged(bool value);

    public slots:
        void itemModifiedChanged(bool) { emit modifiedArtworksCountChanged(); }
        void onFilesUnavailableHandler();
        void onArtworkBackupRequested();
        void onArtworkEditingPaused();
        void onArtworkSpellingInfoUpdated();
        void onUndoStackEmpty();
        void userDictUpdateHandler(const QStringList &keywords, bool overwritten);
        void userDictClearedHandler();

    protected:
        virtual QHash<int, QByteArray> roleNames() const override;
        virtual ArtworkMetadata *createArtwork(const Filesystem::ArtworkFile &file, qint64 directoryID);

    private:
        Models::ArtworkMetadata *accessArtwork(size_t index) const;
        Models::ArtworkMetadata *getArtwork(size_t index) const;
        void destroyArtwork(ArtworkMetadata *artwork);
        int getNextID();

    private:
        template<typename T>
        std::vector<T> selectArtworks(std::function<bool (ArtworkMetadata *)> pred,
                                      std::function<T(ArtworkMetadata *, size_t)> mapper) const {
            std::vector<T> result;
            const size_t size = m_ArtworkList.size();
            for (size_t i = 0; i < size; i++) {
                auto *artwork = accessArtwork(i);
                if (pred(artwork)) {
                    result.push_back(mapper(artwork, i));
                }
            }
            return result;
        }

        template<typename T>
        std::vector<T> selectAvailableArtworks(std::function<T(ArtworkMetadata *, size_t)> mapper) const {
            return selectArtworks([](ArtworkMetadata *artwork) {
                return !artwork->isUnavailable() && !artwork->isRemoved();
            }, mapper);
        }

        void foreachArtwork(std::function<bool (ArtworkMetadata *)> pred,
                            std::function<void (ArtworkMetadata *, size_t)> action) const;
        void foreachArtwork(const Helpers::IndicesRanges &ranges,
                            std::function<void (ArtworkMetadata *, size_t)> action) const;

        template<typename T>
        void foreachArtworkAs(std::function<bool (T *)> pred,
                              std::function<void (T *, size_t)> action) const {
            const size_t size = m_ArtworkList.size();
            for (size_t i = 0; i < size; i++) {
                auto *artwork = accessArtwork(i);
                T *t = dynamic_cast<T*>(artwork);
                if ((t != nullptr) && pred(t)) {
                    action(t, i);
                }
            }
        }

#ifdef INTEGRATION_TESTS
    public:
        void fakeDeleteAllItems();
#endif

    private:
        ArtworksRepository &m_ArtworksRepository;
        ArtworksContainer m_ArtworkList;
        ArtworksContainer m_FinalizationList;
#ifdef QT_DEBUG
        ArtworksContainer m_DestroyedList;
#endif
        qint64 m_LastID;
    };
}

#endif // ARTWORKSLISTMODEL_H
