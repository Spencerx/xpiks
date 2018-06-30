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
#include <Artworks/artworkmetadata.h>
#include <Helpers/ifilenotavailablemodel.h>
#include <Artworks/artworkssnapshot.h>
#include <Filesystem/ifilescollection.h>
#include <Helpers/indicesranges.h>
#include <Commands/Base/icommandtemplate.h>
#include <Commands/appmessages.h>

class QQuickTextDocument;

namespace Commands {
    class ICommand;
}

namespace KeywordsPresets {
    class IPresetsManager;
}

namespace Artworks {
    class IArtworksService;
    class BasicMetadataModel;
}

namespace AutoComplete {
    class ICompletionSource;
}

namespace Services {
    class ArtworkUpdateRequest;
}

namespace Models {
    class ArtworksRepository;

    using IArtworksCommandTemplate = Commands::ICommandTemplate<Artworks::ArtworksSnapshot>;

    class ArtworksListModel:
            public QAbstractListModel,
            public Helpers::IFileNotAvailableModel
    {
        Q_OBJECT
        Q_PROPERTY(int modifiedArtworksCount READ getModifiedArtworksCount NOTIFY modifiedArtworksCountChanged)

        using ArtworksContainer = std::deque<Artworks::ArtworkMetadata *>;

    public:
        ArtworksListModel(ArtworksRepository &repository,
                          Commands::AppMessages &messages,
                          QObject *parent=0);
        virtual ~ArtworksListModel();

    public:
        enum class SelectionType {
            All,
            Modified,
            Selected
        };

    public:
        enum ArtworksListModel_Roles {
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
            ArtworksAddResult(Artworks::ArtworksSnapshot &snapshot, int attachedVectorsCount):
                m_Snapshot(std::move(snapshot)),
                m_AttachedVectorsCount(attachedVectorsCount)
            { }
            Artworks::ArtworksSnapshot m_Snapshot;
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
        QVector<int> getStandardUpdateRoles() const;

    public:
        // general purpose collective methods
        void selectArtworksFromDirectory(int directoryIndex);
        void unselectAllItems();
        void updateItems(const Helpers::IndicesRanges &ranges, const QVector<int> &roles = QVector<int>());
        void updateItems(SelectionType selectionType, const QVector<int> &roles = QVector<int>());
        std::unique_ptr<Artworks::SessionSnapshot> snapshotAll();
        void generateAboutToBeRemoved();
        void unlockAllForIO();
        bool isInSelectedDirectory(int artworkIndex);
        void sendToQuickBuffer(int artworkIndex);

    public:
        // update hub related
        void processUpdateRequests(const std::vector<std::shared_ptr<Services::ArtworkUpdateRequest> > &updateRequests);
        void updateArtworksByIDs(const QSet<qint64> &artworkIDs, const QVector<int> &rolesToUpdate);

    public:
        // add-remove ixtems methods
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
        Artworks::ArtworksSnapshot deleteItems(const Helpers::IndicesRanges &ranges);
        int attachVectors(const std::shared_ptr<Filesystem::IFilesCollection> &filesCollection,
                           const Artworks::ArtworksSnapshot &snapshot,
                           int initialCount,
                           bool autoAttach);
        int attachKnownVectors(const QHash<QString, QHash<QString, QString> > &vectorsPaths,
                               QVector<int> &indicesToUpdate) const;
        void connectArtworkSignals(Artworks::ArtworkMetadata *artwork);
        void syncArtworksIndices(int startIndex=0, int count=-1);
        void setItemsSaved(const Helpers::IndicesRanges &ranges);
        void detachVectorsFromArtworks(const Helpers::IndicesRanges &ranges);
        void setCurrentIndex(size_t index);
        void registerListeners();

    public:
        // qabstractlistmodel methods
        virtual int rowCount(const QModelIndex &) const { return (int)getArtworksCount(); }
        virtual QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) override;

    public:
        Artworks::ArtworkMetadata *getArtworkObject(int index) const;
        Artworks::BasicMetadataModel *getBasicModelObject(int index) const;
        Artworks::ArtworkMetadata *getArtwork(size_t index) const;

    public:
        std::shared_ptr<Commands::ICommand> removeKeywordAt(int artworkIndex, int keywordIndex);
        std::shared_ptr<Commands::ICommand> removeLastKeyword(int artworkIndex);
        std::shared_ptr<Commands::ICommand> appendKeyword(int artworkIndex, const QString &keyword);
        std::shared_ptr<Commands::ICommand> pasteKeywords(int artworkIndex, const QStringList &keywords);
        std::shared_ptr<Commands::ICommand> addSuggestedKeywords(int artworkIndex, const QStringList &keywords);
        std::shared_ptr<Commands::ICommand> editKeyword(int artworkIndex, int keywordIndex, const QString &replacement);
        std::shared_ptr<Commands::ICommand> plainTextEdit(int artworkIndex, const QString &rawKeywords, bool spaceIsSeparator=false);
        std::shared_ptr<Commands::ICommand> expandPreset(int artworkIndex, int keywordIndex,
                                                         unsigned int presetID, KeywordsPresets::IPresetsManager &presetsManager);
        std::shared_ptr<Commands::ICommand> expandLastAsPreset(int artworkIndex,
                                                               KeywordsPresets::IPresetsManager &presetsManager);
        std::shared_ptr<Commands::ICommand> addPreset(int artworkIndex, unsigned int presetID,
                                                      KeywordsPresets::IPresetsManager &presetsManager);
        std::shared_ptr<Commands::ICommand> acceptCompletionAsPreset(int artworkIndex, int completionID,
                                                                     KeywordsPresets::IPresetsManager &presetsManager,
                                                                     AutoComplete::ICompletionSource &completionsSource);
        std::shared_ptr<Commands::ICommand> removeMetadata(const Helpers::IndicesRanges &ranges, Common::ArtworkEditFlags flags);

    signals:
        void modifiedArtworksCountChanged();
        void artworksChanged(bool needToMoveCurrentItem);
        void artworksReimported(int importID, int artworksCount);
        void selectedArtworksRemoved(int count);
        void fileWithIndexUnavailable(size_t index);
        void unavailableArtworksFound();
        void unavailableVectorsFound();
        void artworkSelectedChanged(bool value);

    public slots:
        void onFilesUnavailableHandler();
        void onArtworkEditingPaused();
        void onUndoStackEmpty();
        void onSpellCheckerAvailable(bool afterRestart);
        void userDictUpdateHandler(const QStringList &keywords, bool overwritten);
        void userDictClearedHandler();

    protected:
        virtual QHash<int, QByteArray> roleNames() const override;
        virtual Artworks::ArtworkMetadata *createArtwork(const Filesystem::ArtworkFile &file, qint64 directoryID);

    private:
        Artworks::ArtworkMetadata *accessArtwork(size_t index) const;
        void destroyArtwork(Artworks::ArtworkMetadata *artwork);
        int getNextID();

    private:
        template<typename T>
        std::vector<T> filterArtworks(const Helpers::IndicesRanges &ranges,
                                      std::function<bool (Artworks::ArtworkMetadata *)> pred,
                                      std::function<T(Artworks::ArtworkMetadata *, size_t)> mapper) const {
            std::vector<T> result;
            result.reserve(ranges.size());
            for (auto &r: ranges.getRanges()) {
                for (int i = r.first; i <= r.second; r++) {
                    auto *artwork = accessArtwork(i);
                    if (pred(artwork)) {
                        result.emplace_back(
                                    std::forward<T>(
                                        mapper(artwork, i)));
                    }
                }
            }
            return result;
        }


        template<typename T>
        std::vector<T> filterArtworks(std::function<bool (Artworks::ArtworkMetadata *)> pred,
                                      std::function<T(Artworks::ArtworkMetadata *, size_t)> mapper) const {
            return filterArtworks(Helpers::IndicesRanges(getArtworksCount()),
                                  pred,
                                  mapper);
        }

        template<typename T>
        std::vector<T> filterAvailableArtworks(std::function<T(Artworks::ArtworkMetadata *, size_t)> mapper) const {
            return filterArtworks([](Artworks::ArtworkMetadata *artwork) {
                return !artwork->isUnavailable() && !artwork->isRemoved();
            }, mapper);
        }

        void foreachArtwork(const Helpers::IndicesRanges &ranges,
                            std::function<bool (Artworks::ArtworkMetadata *)> pred,
                            std::function<void (Artworks::ArtworkMetadata *, size_t)> action) const;
        void foreachArtwork(std::function<bool (Artworks::ArtworkMetadata *)> pred,
                            std::function<void (Artworks::ArtworkMetadata *, size_t)> action) const;
        void foreachArtwork(const Helpers::IndicesRanges &ranges,
                            std::function<void (Artworks::ArtworkMetadata *, size_t)> action) const;

        template<typename T>
        void foreachArtworkAs(const Helpers::IndicesRanges &ranges,
                              std::function<bool (T *)> pred,
                              std::function<void (T *, size_t)> action) const {
            for (auto &r: ranges.getRanges()) {
                for (int i = r.first; i <= r.second; r++) {
                    auto *artwork = accessArtwork(i);
                    T *t = dynamic_cast<T*>(artwork);
                    if ((t != nullptr) && pred(t)) {
                        action(t, i);
                    }
                }
            }
        }

#ifdef INTEGRATION_TESTS
    public:
        void fakeDeleteAllItems();
#endif

    private:
        ArtworksContainer m_ArtworkList;
        qint64 m_LastID;
        size_t m_CurrentItemIndex;
        ArtworksRepository &m_ArtworksRepository;
        Commands::AppMessages &m_Messages;
        ArtworksContainer m_FinalizationList;
#ifdef QT_DEBUG
        ArtworksContainer m_DestroyedList;
#endif
    };
}

#endif // ARTWORKSLISTMODEL_H
