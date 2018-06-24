/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef FILTEREDARTITEMSPROXYMODEL_H
#define FILTEREDARTITEMSPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QString>
#include <QList>
#include <functional>
#include "../Common/flags.h"
#include "../Artworks/iartworkssource.h"

namespace Commands {
    class ICommandManager;
}

namespace Models {
    class ArtworkMetadata;
    class ArtworkElement;
    class PreviewArtworkElement;
    class ArtworksListModel;

    class FilteredArtworksListModel:
            public QSortFilterProxyModel,
            public Artworks::IArtworksSource
    {
        Q_OBJECT
        Q_PROPERTY(QString searchTerm READ getSearchTerm WRITE setSearchTerm NOTIFY searchTermChanged)
        Q_PROPERTY(int selectedArtworksCount READ getSelectedArtworksCount NOTIFY selectedArtworksCountChanged)
        Q_PROPERTY(bool s READ getGlobalSelectionChanged NOTIFY allItemsSelectedChanged)

    public:
        FilteredArtworksListModel(ArtworksListModel &artworksListModel,
                                  Commands::ICommandManager &commandManager,
                                  QObject *parent=0);

    public:
        const QString &getSearchTerm() const { return m_SearchTerm; }
        void setSearchTerm(const QString &value);

        int getSelectedArtworksCount() const { return m_SelectedArtworksCount; }
        bool getGlobalSelectionChanged() const { return false; }
        void spellCheckAllItems();

        Artworks::ArtworksSnapshot::Container getSearchablePreviewOriginalItems(const QString &searchTerm, Common::SearchFlags flags) const;

#ifdef CORE_TESTS
        int retrieveNumberOfSelectedItems();
#endif

        // IArtworksSource interface
    public:
        virtual Artworks::WeakArtworksSnapshot getArtworks() override { return getSelectedOriginalItems(); }

    public:
        Q_INVOKABLE int getOriginalIndex(int index) const;
        Q_INVOKABLE int getDerivedIndex(int originalIndex) const;

    public:
        Q_INVOKABLE void invertSelectionArtworks() { invertFilteredItemsSelected(); }
        Q_INVOKABLE void selectFilteredArtworks() { setFilteredItemsSelected(true); }
        Q_INVOKABLE void unselectFilteredArtworks() { setFilteredItemsSelected(false); }
        Q_INVOKABLE void selectDirectory(int directoryIndex);

    public:
        Q_INVOKABLE void removeKeywordAt(int proxyIndex, int keywordIndex);
        Q_INVOKABLE void removeLastKeyword(int proxyIndex);
        Q_INVOKABLE bool appendKeyword(int proxyIndex, const QString &keyword);
        Q_INVOKABLE void pasteKeywords(int proxyIndex, const QStringList &keywords);
        Q_INVOKABLE void addSuggestedKeywords(int proxyIndex, const QStringList &keywords);
        Q_INVOKABLE void editKeyword(int proxyIndex, int keywordIndex, const QString &replacement);
        Q_INVOKABLE void plainTextEdit(int proxyIndex, const QString &rawKeywords, bool spaceIsSeparator=false);
        Q_INVOKABLE void expandPreset(int proxyIndex, int keywordIndex, unsigned int presetID);
        Q_INVOKABLE void expandLastAsPreset(int proxyIndex);
        Q_INVOKABLE void addPreset(int proxyIndex, unsigned int presetID);
        Q_INVOKABLE bool acceptCompletionAsPreset(int proxyIndex, int completionID);

    public:
        Q_INVOKABLE QSize retrieveImageSize(int proxyIndex) const;
        Q_INVOKABLE QString retrieveFileSize(int proxyIndex) const;
        Q_INVOKABLE QString getArtworkFilepath(int proxyIndex) const;
        Q_INVOKABLE QString getAttachedVectorPath(int proxyIndex) const;
        Q_INVOKABLE QString getArtworkDateTaken(int proxyIndex) const;

    public:
        Q_INVOKABLE void setSelectedItemsSaved();
        void removeSelectedArtworks();
        Q_INVOKABLE void updateSelectedArtworks();
        Q_INVOKABLE void saveSelectedArtworks(bool overwriteAll, bool useBackups);
        Q_INVOKABLE void wipeMetadataFromSelectedArtworks(bool useBackups);
        Q_INVOKABLE bool areSelectedArtworksSaved();
        Q_INVOKABLE void spellCheckSelected();
        Q_INVOKABLE int getModifiedSelectedCount(bool overwriteAll=false);
        Q_INVOKABLE void removeArtworksDirectory(int index);
        Q_INVOKABLE void selectArtworksEx(int comboboxSelectionIndex);

        Q_INVOKABLE int getItemsCount() const { return rowCount(); }
        Q_INVOKABLE void reimportMetadataForSelected();
        Q_INVOKABLE int findSelectedItemIndex() const;
        Q_INVOKABLE void removeMetadataInSelected() const;
        Q_INVOKABLE void clearKeywords(int index);

        Q_INVOKABLE void updateFilter() { invalidateFilter(); emit afterInvalidateFilter(); }
        Q_INVOKABLE void focusNextItem(int index);
        Q_INVOKABLE void focusPreviousItem(int index);
        Q_INVOKABLE void focusCurrentItemKeywords(int index);
        Q_INVOKABLE void toggleSorted();
        Q_INVOKABLE void detachVectorFromSelected();
        Q_INVOKABLE void detachVectorFromArtwork(int index);
        Q_INVOKABLE QObject *getArtworkMetadata(int index);
        Q_INVOKABLE QObject *getBasicModel(int index);
        Q_INVOKABLE QString getKeywordsString(int index);

        Q_INVOKABLE bool hasTitleWordSpellError(int index, const QString &word);
        Q_INVOKABLE bool hasDescriptionWordSpellError(int index, const QString &word);

        Q_INVOKABLE void registerCurrentItem(int index) const;
        Q_INVOKABLE void copyToQuickBuffer(int index) const;
        Q_INVOKABLE void fillFromQuickBuffer(int index) const;
        Q_INVOKABLE void suggestCorrectionsForSelected() const;
        Q_INVOKABLE void generateCompletions(const QString &prefix, int index);
        Q_INVOKABLE void reviewDuplicatesInSelected() const;

    public slots:
        void itemSelectedChanged(bool value);
        void onSelectedArtworksRemoved(int value);
        void onSpellCheckerAvailable(bool afterRestart);
        void onSettingsUpdated();
        void onDirectoriesSelectionChanged() { updateFilter(); }

    signals:
        void searchTermChanged(const QString &searchTerm);
        void selectedArtworksCountChanged();
        void afterInvalidateFilter();
        void allItemsSelectedChanged();

    private:
        void removeMetadataInItems(Artworks::ArtworksSnapshot::Container &itemsToClear, Common::ArtworkEditFlags flags) const;
        void removeKeywordsInItem(ArtworkMetadata *artwork);
        void setFilteredItemsSelected(bool selected);
        void setFilteredItemsSelectedEx(const std::function<bool (ArtworkMetadata *)> pred, bool selected, bool unselectFirst);
        void invertFilteredItemsSelected();

        Artworks::WeakArtworksSnapshot getSelectedOriginalItems() const;
        Artworks::ArtworksSnapshot::Container getSelectedArtworksSnapshot() const;

        template<typename T>
        std::vector<T> getFilteredOriginalItems(std::function<bool (ArtworkMetadata *)> pred,
                                                std::function<T(ArtworkMetadata *, int, int)> mapper) const;

        Artworks::WeakArtworksSnapshot getAllOriginalItems() const;

        std::vector<int> getSelectedOriginalIndices() const;
        std::vector<int> getSelectedIndices() const;
        void forceUnselectAllItems();
        void updateSearchFlags();

    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
        virtual bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;

    private:
        ArtworksListModel &m_ArtworksListModel;
        Commands::ICommandManager &m_CommandManager;
        // ignore default regexp from proxymodel
        QString m_SearchTerm;
        Common::SearchFlags m_SearchFlags;
        volatile int m_SelectedArtworksCount;
        volatile bool m_SortingEnabled;
    };
}

#endif // FILTEREDARTITEMSPROXYMODEL_H
