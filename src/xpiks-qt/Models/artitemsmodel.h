/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTSITEMSMODEL_H
#define ARTSITEMSMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QList>
#include <QVector>
#include <QUrl>
#include <QSize>
#include <QHash>
#include <QQuickTextDocument>

#include <deque>
#include <vector>
#include <tuple>
#include <memory>

#include "../Common/ibasicartwork.h"
#include "../Common/iartworkssource.h"
#include "../KeywordsPresets/ipresetsmanager.h"
#include "../Helpers/ifilenotavailablemodel.h"
#include "artworksrepository.h"
#include "../MetadataIO/artworkssnapshot.h"
#include "../Filesystem/ifilescollection.h"
#include "../Helpers/indicesranges.h"

namespace Common {
    class BasicMetadataModel;
}

namespace QMLExtensions {
    class ArtworkUpdateRequest;
}

namespace Models {
    class ArtworkMetadata;
    class ArtworkElement;

    class ArtItemsModel:
            public QAbstractListModel,
            public Common::IArtworksSource,
            public Helpers::IFileNotAvailableModel
    {

    public:

        ArtworksRemoveResult removeFiles(const Helpers::IndicesRanges &ranges);



    private:
        void removeArtworks(const Helpers::IndicesRanges &ranges);


    public:

    public:
        void updateItems(const QVector<int> &indices, const QVector<int> &roles);
        void forceUnselectAllItems() const;
        virtual bool removeUnavailableItems() override;
        void generateAboutToBeRemoved();

    public:
        Q_INVOKABLE void updateAllItems();
        /*Q_INVOKABLE*/ void removeArtworksDirectory(int index);
        Q_INVOKABLE void removeKeywordAt(int metadataIndex, int keywordIndex);
        Q_INVOKABLE void removeLastKeyword(int metadataIndex);
        Q_INVOKABLE bool appendKeyword(int metadataIndex, const QString &keyword);
        Q_INVOKABLE void pasteKeywords(int metadataIndex, const QStringList &keywords);
        Q_INVOKABLE void addSuggestedKeywords(int metadataIndex, const QStringList &keywords);
        Q_INVOKABLE void suggestCorrections(int metadataIndex);
        Q_INVOKABLE void backupItem(int metadataIndex);

        Q_INVOKABLE void combineArtwork(int index) { doCombineArtwork(index); }

        /*Q_INVOKABLE*/ void setSelectedItemsSaved(const QVector<int> &selectedIndices);

        /*Q_INVOKABLE*/ void updateSelectedArtworks(const QVector<int> &selectedIndices);
        /*Q_INVOKABLE*/ void updateSelectedArtworksEx(const QVector<int> &selectedIndices, const QVector<int> roles);

        /*Q_INVOKABLE*/ ArtworkMetadata *getArtworkMetadata(int index) const;

        /*Q_INVOKABLE*/ Common::BasicMetadataModel *getBasicModel(int index) const;

        Q_INVOKABLE QSize retrieveImageSize(int metadataIndex) const;
        Q_INVOKABLE QString retrieveFileSize(int metadataIndex) const;
        Q_INVOKABLE QString getArtworkFilepath(int index) const;
        Q_INVOKABLE QString getAttachedVectorPath(int metadataIndex) const;
        Q_INVOKABLE QString getArtworkDateTaken(int metadataIndex) const;

        Q_INVOKABLE int addRecentDirectory(const QString &directory);
        Q_INVOKABLE int addRecentFile(const QString &file);
        Q_INVOKABLE int addAllRecentFiles();
        Q_INVOKABLE void initDescriptionHighlighting(int metadataIndex, QQuickTextDocument *document);
        Q_INVOKABLE void initTitleHighlighting(int metadataIndex, QQuickTextDocument *document);

        Q_INVOKABLE void editKeyword(int metadataIndex, int keywordIndex, const QString &replacement);
        Q_INVOKABLE void plainTextEdit(int metadataIndex, const QString &rawKeywords, bool spaceIsSeparator=false);

        /*Q_INVOKABLE*/ void detachVectorsFromArtworks(const QVector<int> &indices);

        Q_INVOKABLE void expandPreset(int artworkIndex, int keywordIndex, unsigned int presetID);
        Q_INVOKABLE void expandLastAsPreset(int metadataIndex);
        Q_INVOKABLE void addPreset(int metadataIndex, unsigned int presetID);
        Q_INVOKABLE bool acceptCompletionAsPreset(int metadataIndex, int completionID);
        Q_INVOKABLE void initSuggestion(int metadataIndex);
        Q_INVOKABLE void setupDuplicatesModel(int metadataIndex);
        Q_INVOKABLE bool hasModifiedArtworks() const;

    public:
        void fillFromQuickBuffer(size_t metadataIndex);

    public:
        void syncArtworksIndices();
        void insertArtwork(int index, ArtworkMetadata *artwork);
        void appendArtwork(ArtworkMetadata *artwork);
        ArtworkMetadata *getArtwork(size_t index) const;
        void raiseArtworksReimported(int importID, int artworksCount);
        void raiseArtworksChanged(bool navigateToCurrent);
        virtual void updateItemsAtIndices(const QVector<int> &indices);
        void updateItemsAtIndicesEx(const QVector<int> &indices, const QVector<int> &roles);
        virtual void updateItemsInRanges(const QVector<QPair<int, int> > &ranges);
        void updateItemsInRangesEx(const QVector<QPair<int, int> > &ranges, const QVector<int> &roles);
        void setAllItemsSelected(bool selected);
        void unlockAllForIO();
        void resetSpellCheckResults();
        void resetDuplicatesInfo();

    public:
        // update hub related
        void processUpdateRequests(const std::vector<std::shared_ptr<QMLExtensions::ArtworkUpdateRequest> > &updateRequests);
        void updateArtworks(const QSet<qint64> &artworkIDs, const QVector<int> &rolesToUpdate);

    public:
        // IARTWORKSSOURCE
        virtual Common::IBasicArtwork *getBasicArtwork(int index) const override;
        virtual int getArtworksCount() const override { return (int)m_ArtworkList.size(); }

    private:
        void updateItemAtIndex(int metadataIndex);

    private:
        void doCombineArtwork(int index);

    private:
        void destroyInnerItem(ArtworkMetadata *artwork);
        void doRemoveItemsFromRanges(QVector<int> &indicesToRemove, bool isFullDirectory = false);
        void doRemoveItemsInRanges(const QVector<QPair<int, int> > &rangesToRemove, bool isFullDirectory = false);
        void getSelectedItemsIndices(QVector<int> &indices);

    public:
        QVector<int> getArtworkStandardRoles() const;

    private:
        void fillStandardRoles(QVector<int> &roles) const;

#ifdef CORE_TESTS
    public:
        const ArtworksContainer &getFinalizationList() const { return m_FinalizationList; }
#endif

#ifdef INTEGRATION_TESTS
    public:
        ArtworkMetadata *findArtworkByFilepath(const QString &filepath);
#endif

    public:
        const ArtworksContainer &getArtworkList() const { return m_ArtworkList; }


}

#endif // ARTSITEMSMODEL_H
