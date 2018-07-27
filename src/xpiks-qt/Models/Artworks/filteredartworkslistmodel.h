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
#include <Common/flags.h>
#include <Artworks/artworkssnapshot.h>
#include <Artworks/iselectedartworkssource.h>
#include <Artworks/iselectedindicessource.h>
#include <Models/Editing/quickbuffermessage.h>
#include <Common/messages.h>

namespace Commands {
    class ICommandManager;
}

namespace KeywordsPresets {
    class IPresetsManager;
}

namespace AutoComplete {
    class ICompletionSource;
}

namespace Models {
    class ArtworkMetadata;
    class ArtworkElement;
    class PreviewArtworkElement;
    class ArtworksListModel;
    class SettingsModel;

    class FilteredArtworksListModel:
            public QSortFilterProxyModel,
            public Artworks::ISelectedArtworksSource,
            public Artworks::ISelectedIndicesSource,
            public Common::MessagesSource<QuickBufferMessage>
    {
        Q_OBJECT
        Q_PROPERTY(QString searchTerm READ getSearchTerm WRITE setSearchTerm NOTIFY searchTermChanged)
        Q_PROPERTY(int selectedArtworksCount READ getSelectedArtworksCount NOTIFY selectedArtworksCountChanged)
        Q_PROPERTY(bool s READ getGlobalSelectionChanged NOTIFY allItemsSelectedChanged)

    public:
        FilteredArtworksListModel(ArtworksListModel &artworksListModel,
                                  Commands::ICommandManager &commandManager,
                                  KeywordsPresets::IPresetsManager &presetsManager,
                                  AutoComplete::ICompletionSource &completionSource,
                                  Models::SettingsModel &settingsModel,
                                  QObject *parent=0);

    public:
        const QString &getSearchTerm() const { return m_SearchTerm; }
        void setSearchTerm(const QString &value);

        int getSelectedArtworksCount() const { return m_SelectedArtworksCount; }
        bool getGlobalSelectionChanged() const { return false; }

        Artworks::ArtworksSnapshot::Container getSearchablePreviewOriginalItems(const QString &searchTerm, Common::SearchFlags flags) const;
        Artworks::ArtworksSnapshot getArtworksToSave(bool overwriteAll=false) const;

    public:
        // returns currently selected artworks as a snapshot
        virtual Artworks::ArtworksSnapshot getSelectedArtworks() override;
        virtual std::vector<int> getSelectedIndices() override { return getSelectedOriginalIndices(); }

#ifdef CORE_TESTS
        int retrieveNumberOfSelectedItems();
#endif

    public:
        // indexing
        Q_INVOKABLE int getOriginalIndex(int proxyIndex) const;
        Q_INVOKABLE int getDerivedIndex(int originalIndex) const;
        Q_INVOKABLE QObject *getArtworkObject(int proxyIndex);
        Q_INVOKABLE QObject *getBasicModelObject(int proxyIndex);

    public:
        // selection
        Q_INVOKABLE void invertSelectionArtworks();
        Q_INVOKABLE void selectFilteredArtworks() { setFilteredItemsSelected(true); }
        Q_INVOKABLE void unselectFilteredArtworks() { setFilteredItemsSelected(false); }
        Q_INVOKABLE void selectDirectory(int directoryIndex);
        Q_INVOKABLE void setSelectedItemsSaved();
        Q_INVOKABLE bool areSelectedArtworksSaved();
        Q_INVOKABLE int getModifiedSelectedCount(bool overwriteAll=false);
        Q_INVOKABLE int findSelectedItemIndex() const;
        Q_INVOKABLE void selectArtworksEx(int comboboxSelectionIndex);

    public:
        // ui actions
        Q_INVOKABLE void focusNextItem(int proxyIndex);
        Q_INVOKABLE void focusPreviousItem(int proxyIndex);
        Q_INVOKABLE void focusCurrentItemKeywords(int proxyIndex);
        Q_INVOKABLE void toggleSorted();

    public:
        // edit actions
        Q_INVOKABLE void removeKeywordAt(int proxyIndex, int keywordIndex);
        Q_INVOKABLE void removeLastKeyword(int proxyIndex);
        Q_INVOKABLE void appendKeyword(int proxyIndex, const QString &keyword);
        Q_INVOKABLE void pasteKeywords(int proxyIndex, const QStringList &keywords);
        Q_INVOKABLE void addSuggestedKeywords(int proxyIndex, const QStringList &keywords);
        Q_INVOKABLE void editKeyword(int proxyIndex, int keywordIndex, const QString &replacement);
        Q_INVOKABLE void plainTextEdit(int proxyIndex, const QString &rawKeywords, bool spaceIsSeparator=false);
        Q_INVOKABLE void expandPreset(int proxyIndex, int keywordIndex, unsigned int presetID);
        Q_INVOKABLE void expandLastAsPreset(int proxyIndex);
        Q_INVOKABLE void addPreset(int proxyIndex, unsigned int presetID);
        Q_INVOKABLE void acceptCompletionAsPreset(int proxyIndex, int completionID);

    public:
        // other modifications
        Q_INVOKABLE void removeMetadataInSelected() const;
        Q_INVOKABLE void clearKeywords(int proxyIndex);
        Q_INVOKABLE void detachVectorFromSelected();
        Q_INVOKABLE void detachVectorFromArtwork(int proxyIndex);

    public:
        // other readonly
        Q_INVOKABLE int getItemsCount() const { return rowCount(); }
        Q_INVOKABLE void updateFilter() { invalidateFilter(); emit afterInvalidateFilter(); }
        Q_INVOKABLE QString getKeywordsString(int proxyIndex);
        Q_INVOKABLE bool hasTitleWordSpellError(int proxyIndex, const QString &word);
        Q_INVOKABLE bool hasDescriptionWordSpellError(int proxyIndex, const QString &word);

    public:
        // other
        Q_INVOKABLE void registerCurrentItem(int proxyIndex) const;

    public:
        //Q_INVOKABLE void removeArtworksDirectory(int index);

        Q_INVOKABLE void copyToQuickBuffer(int proxyIndex) const;
        //Q_INVOKABLE void fillFromQuickBuffer(int index) const;
        //Q_INVOKABLE void generateCompletions(const QString &prefix, int index);

    public slots:
        void itemSelectedChanged(bool value);
        void onSelectedArtworksRemoved(int value);
        void onSettingsUpdated();
        void onDirectoriesSelectionChanged() { updateFilter(); }

    signals:
        void searchTermChanged(const QString &searchTerm);
        void selectedArtworksCountChanged();
        void afterInvalidateFilter();
        void allItemsSelectedChanged();
        void clearCurrentEditable();

    private:
        void setFilteredItemsSelected(bool selected);
        void setFilteredItemsSelectedEx(const std::function<bool (Artworks::ArtworkMetadata *)> pred,
                                        bool selected,
                                        bool unselectAllFirst);

        Artworks::WeakArtworksSnapshot getSelectedOriginalItems() const;

        template<typename T>
        std::vector<T> filterItems(std::function<bool (Artworks::ArtworkMetadata *)> pred,
                                   std::function<T(Artworks::ArtworkMetadata *, int, int)> mapper) const;

        std::vector<int> getSelectedOriginalIndices() const;
        void forceUnselectAllItems();
        void updateSearchFlags();

    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
        virtual bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;

    private:
        ArtworksListModel &m_ArtworksListModel;
        Commands::ICommandManager &m_CommandManager;
        KeywordsPresets::IPresetsManager &m_PresetsManager;
        AutoComplete::ICompletionSource &m_CompletionSource;
        Models::SettingsModel &m_SettingsModel;
        // ignore default regexp from proxymodel
        QString m_SearchTerm;
        Common::SearchFlags m_SearchFlags;
        volatile int m_SelectedArtworksCount;
        volatile bool m_SortingEnabled;
    };
}

#endif // FILTEREDARTITEMSPROXYMODEL_H
