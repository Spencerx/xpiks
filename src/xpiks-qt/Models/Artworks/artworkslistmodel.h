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

#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include <QAbstractListModel>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>
#include <Qt>
#include <QtGlobal>

#include "Artworks/artworkmetadata.h"
#include "Artworks/artworkssnapshot.h"
#include "Common/flags.h"
#include "Common/messages.h"
#include "Common/types.h"
#include "Helpers/indicesranges.h"
#include "artworkslistoperations.h"

class QByteArray;
class QModelIndex;

namespace Artworks {
    class BasicMetadataModel;
    class IBasicModelSource;
}

namespace AutoComplete {
    class ICompletionSource;
}

namespace Commands {
    class ICommand;
}

namespace Filesystem {
    class IFilesCollection;
    struct ArtworkFile;
}

namespace KeywordsPresets {
    class IPresetsManager;
}

namespace Services {
    class ArtworkUpdateRequest;
}

namespace Models {
    class ArtworksRepository;
    class ICurrentEditable;

    using ArtworkSpellCheckMessage = Common::NamedType<std::shared_ptr<Artworks::ArtworkMetadata>, Common::MessageType::EditingPaused>;
    using ArtworksListSpellCheckMessage = Common::NamedType<std::vector<std::shared_ptr<Artworks::IBasicModelSource>>, Common::MessageType::SpellCheck>;
    using UnavailableFilesMessage = Common::NamedType<int, Common::MessageType::UnavailableFiles>;

    class ArtworksListModel:
            public QAbstractListModel,
            public Common::MessagesSource<ArtworkSpellCheckMessage>,
            public Common::MessagesSource<std::shared_ptr<ICurrentEditable>>,
            public Common::MessagesSource<ArtworksListSpellCheckMessage>,
            public Common::MessagesSource<UnavailableFilesMessage>
    {
        Q_OBJECT
        Q_PROPERTY(int modifiedArtworksCount READ getModifiedArtworksCount NOTIFY modifiedArtworksCountChanged)

        using Common::MessagesSource<ArtworkSpellCheckMessage>::sendMessage;
        using Common::MessagesSource<std::shared_ptr<ICurrentEditable>>::sendMessage;
        using Common::MessagesSource<ArtworksListSpellCheckMessage>::sendMessage;
        using Common::MessagesSource<UnavailableFilesMessage>::sendMessage;

    public:
        ArtworksListModel(ArtworksRepository &repository,
                          QObject *parent=0);
        virtual ~ArtworksListModel();

    public:
        using ArtworkItem = std::shared_ptr<Artworks::ArtworkMetadata>;

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

    public:
        size_t getArtworksSize() const { return m_ArtworkList.size(); }
        int getModifiedArtworksCount();
        QVector<int> getStandardUpdateRoles() const;

    public:
        // general purpose collective methods
        void selectArtworksFromDirectory(int directoryIndex);
        void unselectAllItems();
        virtual void updateItems(const Helpers::IndicesRanges &ranges, const QVector<int> &roles = QVector<int>());
        void updateSelection(SelectionType selectionType, const QVector<int> &roles = QVector<int>());
        std::unique_ptr<Artworks::SessionSnapshot> createSessionSnapshot();
        Artworks::ArtworksSnapshot createArtworksSnapshot();
        void generateAboutToBeRemoved();
        void unlockAllForIO();
        bool isInSelectedDirectory(int artworkIndex);
        void setCurrentIndex(size_t index);
        void unsetCurrentIndex();
        void setItemsSaved(const Helpers::IndicesRanges &ranges);
        void detachVectorsFromArtworks(const Helpers::IndicesRanges &ranges);
        void purgeUnavailableFiles();

    public:
        // update hub related
        void processUpdateRequests(const std::vector<std::shared_ptr<Services::ArtworkUpdateRequest> > &updateRequests);
        void updateArtworksByIDs(const QSet<qint64> &artworkIDs, const QVector<int> &rolesToUpdate);

    public:
        // add-remove items methods
        ArtworksAddResult addFiles(const std::shared_ptr<Filesystem::IFilesCollection> &filesCollection,
                                   Common::AddFilesFlags flags);
        Artworks::ArtworksSnapshot addArtworks(std::shared_ptr<Filesystem::IFilesCollection> const &filesCollection,
                                               QSet<qint64> &fullDirectories);
        ArtworksRemoveResult removeFiles(Helpers::IndicesRanges const &ranges);
        ArtworksRemoveResult removeFilesFromDirectory(int directoryIndex);
        void restoreRemoved(Helpers::IndicesRanges const &ranges);
        void deleteRemovedItems();
        void deleteAllItems();

    protected:
        void deleteItems(const Helpers::IndicesRanges &ranges);

    private:
        void deleteUnavailableItems();
        // general purpose internal methods
        int attachVectors(const std::shared_ptr<Filesystem::IFilesCollection> &filesCollection,
                          const Artworks::ArtworksSnapshot &snapshot,
                          int initialCount,
                          bool autoAttach);
        int attachKnownVectors(const QHash<QString, QHash<QString, QString> > &vectorsPaths,
                               QVector<int> &indicesToUpdate) const;
        void connectArtworkSignals(Artworks::ArtworkMetadata *artwork);
        void syncArtworksIndices(int startIndex=0, int count=-1);

    private:
        // message handlers
        void resetSpellCheckResults();
        void resetDuplicatesResults();
        void spellCheckAll();

    public:
        // qabstractlistmodel methods
        virtual int rowCount(const QModelIndex &) const override { return (int)getArtworksSize(); }
        virtual QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) override;

    public:
        Artworks::ArtworkMetadata *getArtworkObject(int index) const;
        Artworks::BasicMetadataModel *getBasicModelObject(int index) const;
        bool tryGetArtwork(size_t index, ArtworkItem &item) const;

    public:
        std::shared_ptr<Commands::ICommand> removeKeywordAt(int artworkIndex, int keywordIndex);
        std::shared_ptr<Commands::ICommand> removeLastKeyword(int artworkIndex);
        std::shared_ptr<Commands::ICommand> appendKeyword(int artworkIndex, const QString &keyword);
        std::shared_ptr<Commands::ICommand> pasteKeywords(int artworkIndex, const QStringList &keywords);
        std::shared_ptr<Commands::ICommand> editKeyword(int artworkIndex, int keywordIndex, const QString &replacement);
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
        void selectedArtworksRemoved(int count);
        void unavailableArtworksFound();
        void unavailableVectorsFound();
        void artworkSelectedChanged(bool value);

    public slots:
        void onFilesUnavailableHandler();
        void onArtworkEditingPaused();
        void onUndoStackEmpty();
        void onSpellCheckerAvailable();
        void onSpellCheckDisabled();
        void onDuplicatesDisabled();
        void userDictUpdateHandler(const QStringList &keywords, bool overwritten);
        void userDictClearedHandler();
        void onMetadataWritingFinished();

    protected:
        virtual QHash<int, QByteArray> roleNames() const override;
        virtual ArtworkItem createArtwork(const Filesystem::ArtworkFile &file, qint64 directoryID);
        std::shared_ptr<Artworks::ArtworkMetadata> const &accessArtwork(size_t index) const;

    private:
        void destroyArtwork(ArtworkItem const &artwork);
        int getNextID();

    private:
        template<typename T>
        std::vector<T> filterArtworks(const Helpers::IndicesRanges &ranges,
                                      std::function<bool (ArtworkItem const &)> pred,
                                      std::function<T(ArtworkItem const &, size_t)> mapper) const {
            std::vector<T> result;
            result.reserve(ranges.size());
            for (auto &r: ranges.getRanges()) {
                for (int i = r.first; i <= r.second; i++) {
                    auto &artwork = accessArtwork(i);
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
        std::vector<T> filterArtworks(std::function<bool (ArtworkItem const &)> pred,
                                      std::function<T(ArtworkItem const &, size_t)> mapper) const {
            return filterArtworks(Helpers::IndicesRanges(getArtworksSize()),
                                  pred,
                                  mapper);
        }

        int foreachArtwork(const Helpers::IndicesRanges &ranges,
                           std::function<bool (ArtworkItem const &)> pred,
                           std::function<void (ArtworkItem const &, size_t)> action) const;
        int foreachArtwork(std::function<bool (ArtworkItem const &)> pred,
                           std::function<void (ArtworkItem const &, size_t)> action) const;
        int foreachArtwork(const Helpers::IndicesRanges &ranges,
                           std::function<void (ArtworkItem const &, size_t)> action) const;

        template<typename T>
        void foreachArtworkAs(const Helpers::IndicesRanges &ranges,
                              std::function<bool (std::shared_ptr<T> const &)> pred,
                              std::function<void (std::shared_ptr<T> const &, size_t)> action) const {
            for (auto &r: ranges.getRanges()) {
                for (int i = r.first; i <= r.second; i++) {
                    auto &artwork = accessArtwork(i);
                    std::shared_ptr<T> t = std::dynamic_pointer_cast<T>(artwork);
                    if ((t != nullptr) && pred(t)) {
                        action(t, i);
                    }
                }
            }
        }

    protected:
        template<typename T>
        std::vector<T> filterAvailableArtworks(std::function<T(ArtworkItem const &, size_t)> mapper) const {
            return filterArtworks([](ArtworkItem const &artwork) {
                return !artwork->isUnavailable() && !artwork->isRemoved();
            }, mapper);
        }

    private:
        std::deque<std::shared_ptr<Artworks::ArtworkMetadata>> m_ArtworkList;
        qint64 m_LastID;
        size_t m_CurrentItemIndex;
        ArtworksRepository &m_ArtworksRepository;
    };
}

#endif // ARTWORKSLISTMODEL_H
