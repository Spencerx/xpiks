/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "filteredartworkslistmodel.h"
#include <QDir>
#include "artworkslistmodel.h"
#include <Common/flags.h>
#include <Common/defines.h>
#include <Commands/commandmanager.h>
#include <Commands/Editing/modifyartworkscommand.h>
#include <Artworks/artworkmetadata.h>
#include <Artworks/artworkelement.h>
#include <Artworks/videoartwork.h>
#include <Helpers/indiceshelper.h>
#include <Helpers/filterhelpers.h>
#include <Models/Editing/previewartworkelement.h>
#include <Models/settingsmodel.h>

namespace Models {    
    FilteredArtworksListModel::FilteredArtworksListModel(ArtworksListModel &artworksListModel,
                                                         Commands::ICommandManager &commandManager,
                                                         KeywordsPresets::IPresetsManager &presetsManager,
                                                         SettingsModel &settingsModel,
                                                         QObject *parent):
        QSortFilterProxyModel(parent),
        m_ArtworksListModel(artworksListModel),
        m_CommandManager(commandManager),
        m_PresetsManager(presetsManager),
        m_SettingsModel(settingsModel),
        m_SelectedArtworksCount(0),
        m_SortingEnabled(false)
    {
        // m_SortingEnabled = true;
        // this->sort(0);
        m_SearchFlags = Common::SearchFlags::AnyTermsEverything;

        QObject::connect(&m_ArtworksListModel, &ArtworksListModel::artworksChanged,
                         this, &FilteredArtworksListModel::onArtworksChanged);

        setSourceModel(&m_ArtworksListModel);
    }

    void FilteredArtworksListModel::setSearchTerm(const QString &value) {
        LOG_INFO << value;
        bool anyChangesNeeded = value != m_SearchTerm;

        if (anyChangesNeeded) {
            m_SearchTerm = value;
            emit searchTermChanged(value);
        }

        updateSearchFlags();

        updateFilter();
        forceUnselectAllItems();
    }

    Artworks::ArtworksSnapshot FilteredArtworksListModel::getArtworksToSave(bool overwriteAll) const {
        auto rawSnapshot = filterItems<std::shared_ptr<Artworks::ArtworkMetadata>>(
                    [overwriteAll](ArtworkItem const &artwork) {
                       return artwork->isSelected() && !artwork->isReadOnly() && (artwork->isModified() || overwriteAll); },
                [] (ArtworkItem const &artwork, int, int) { return artwork; });

        return Artworks::ArtworksSnapshot(rawSnapshot);
    }

    bool FilteredArtworksListModel::tryGetArtwork(int proxyIndex, std::shared_ptr<Artworks::ArtworkMetadata> &artwork) {
        int originalIndex = getOriginalIndex(proxyIndex);
        return m_ArtworksListModel.tryGetArtwork(originalIndex, artwork);
    }

    Artworks::ArtworksSnapshot FilteredArtworksListModel::getSelectedArtworks() {
        auto rawSnapshot = filterItems<std::shared_ptr<Artworks::ArtworkMetadata>>(
                    [](ArtworkItem const &artwork) { return artwork->isSelected(); },
                [] (ArtworkItem const &artwork, int, int) { return artwork; });

        return Artworks::ArtworksSnapshot(rawSnapshot);
    }

    void FilteredArtworksListModel::acceptCompletionAsPreset(int proxyIndex, AutoComplete::ICompletionSource &completionsSource, int completionID) {
        LOG_INFO << "proxyIndex:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        auto command = m_ArtworksListModel.acceptCompletionAsPreset(originalIndex, completionID,
                                                                    m_PresetsManager, completionsSource);
        m_CommandManager.processCommand(command);
    }

    int FilteredArtworksListModel::getOriginalIndex(int proxyIndex) const {
        QModelIndex originalIndex = mapToSource(this->index(proxyIndex, 0));
        int row = originalIndex.row();
        return row;
    }

    int FilteredArtworksListModel::getDerivedIndex(int originalIndex) const {
        QModelIndex index = mapFromSource(m_ArtworksListModel.index(originalIndex, 0));
        int row = index.row();

        return row;
    }

    QObject *FilteredArtworksListModel::getArtworkObject(int proxyIndex) {
        int originalIndex = getOriginalIndex(proxyIndex);
        QObject *item = m_ArtworksListModel.getArtworkObject(originalIndex);
        return item;
    }

    QObject *FilteredArtworksListModel::getBasicModelObject(int proxyIndex) {
        int originalIndex = getOriginalIndex(proxyIndex);
        QObject *item = m_ArtworksListModel.getBasicModelObject(originalIndex);
        return item;
    }

    void FilteredArtworksListModel::invertSelectionArtworks() {
        LOG_DEBUG << "#";

        std::vector<int> indices;
        const int size = this->rowCount();
        indices.reserve(size);

        for (int row = 0; row < size; ++row) {
            int index = getOriginalIndex(row);
            auto &artwork = m_ArtworksListModel.getArtwork(index);
            if (artwork != nullptr) {
                artwork->invertSelection();
                indices.push_back(index);
            }
        }

        m_ArtworksListModel.updateItems(Helpers::IndicesRanges(indices),
                                        QVector<int>() << ArtworksListModel::IsSelectedRole);
        emit allItemsSelectedChanged();
    }

    void FilteredArtworksListModel::selectDirectory(int directoryIndex) {
        LOG_DEBUG << "#";
        m_ArtworksListModel.selectArtworksFromDirectory(directoryIndex);
        emit allItemsSelectedChanged();
    }

    void FilteredArtworksListModel::setSelectedItemsSaved() {
        LOG_DEBUG << "#";
        std::vector<int> indices = getSelectedOriginalIndices();
        Helpers::IndicesRanges ranges(indices);
        m_ArtworksListModel.setItemsSaved(ranges);
    }

    bool FilteredArtworksListModel::areSelectedArtworksSaved() {
        auto selectedArtworks = getSelectedOriginalItems();
        bool anyModified = false;

        for (auto *artwork: selectedArtworks) {
            if (artwork->isModified()) {
                anyModified = true;
                break;
            }
        }

        LOG_DEBUG << "any modified:" << anyModified;

        return !anyModified;
    }

    int FilteredArtworksListModel::getModifiedSelectedCount(bool overwriteAll) {
        auto selectedArtworks = getSelectedOriginalItems();
        int modifiedCount = 0;

        for (auto &artwork: selectedArtworks) {
            if (!artwork->isReadOnly() && (artwork->isModified() || overwriteAll)) {
                modifiedCount++;
            }
        }

        return modifiedCount;
    }

    int FilteredArtworksListModel::findSelectedItemIndex() const {
        int index = -1;

        std::vector<int> items = filterItems<int>(
            [](ArtworkItem const &artwork) { return artwork->isSelected(); },
            [] (ArtworkItem const &, int, int originalIndex) { return originalIndex; });

        if (items.size() == 1) {
            index = items.front();
        }

        return index;
    }

    void FilteredArtworksListModel::selectArtworksEx(int comboboxSelectionIndex) {
        const bool isSelected = true;
        const bool unselectFirst = true;

        switch(comboboxSelectionIndex) {
        case 0: {
            // select All
            this->selectFilteredArtworks();
            break;
        }
        case 1: {
            // select None
            this->unselectFilteredArtworks();
            break;
        }
        case 2: {
            // select Modified
            this->setFilteredItemsSelectedEx([](ArtworkItem const &artwork) {
                return artwork->isModified();
            }, isSelected, unselectFirst);
            break;
        }
        case 3: {
            // select Images
            this->setFilteredItemsSelectedEx([](ArtworkItem const &artwork) {
                auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
                return (image != nullptr) ? !image->hasVectorAttached() : false;
            }, isSelected, unselectFirst);
            break;
        }
        case 4: {
            // select Vectors
            this->setFilteredItemsSelectedEx([](ArtworkItem const &artwork) {
                Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork*>(artwork);
                return (image != nullptr) ? image->hasVectorAttached() : false;
            }, isSelected, unselectFirst);
            break;
        }
        case 5: {
            // select Videos
            this->setFilteredItemsSelectedEx([](ArtworkItem const &artwork) {
                return dynamic_cast<Artworks::VideoArtwork*>(artwork) != nullptr;
            }, isSelected, unselectFirst);
            break;
        }
        default:
            break;
        }
    }

    void FilteredArtworksListModel::focusNextItem(int proxyIndex) {
        LOG_INFO << "index:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex + 1);
        ArtworksListModel::ArtworkItem artwork;
        if (m_ArtworksListModel.tryGetArtwork(originalIndex, artwork)) {
            artwork->requestFocus(Artworks::ArtworkMetadata::FocusFromPrev);
        }
    }

    void FilteredArtworksListModel::focusPreviousItem(int proxyIndex) {
        LOG_INFO << "index:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex - 1);
        ArtworksListModel::ArtworkItem artwork;
        if (m_ArtworksListModel.tryGetArtwork(originalIndex, artwork)) {
            artwork->requestFocus(Artworks::ArtworkMetadata::FocusFromNext);
        }
    }

    void FilteredArtworksListModel::focusCurrentItemKeywords(int proxyIndex) {
        LOG_INFO << "index:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        ArtworksListModel::ArtworkItem artwork;
        if (m_ArtworksListModel.tryGetArtwork(originalIndex, artwork)) {
            artwork->requestFocus(Artworks::ArtworkMetadata::FocusFromNext);
        }
    }

    void FilteredArtworksListModel::toggleSorted() {
        LOG_INFO << "current sorted is" << m_SortingEnabled;
        forceUnselectAllItems();

        if (!m_SortingEnabled) {
            m_SortingEnabled = true;
            sort(0);
            invalidate();
        } else {
            m_SortingEnabled = false;
            setSortRole(Qt::InitialSortOrderRole);
            sort(-1);
            invalidate();
        }

        m_ArtworksListModel.updateSelection(ArtworksListModel::SelectionType::All);
    }

    void FilteredArtworksListModel::removeKeywordAt(int proxyIndex, int keywordIndex) {
        LOG_INFO << "proxyIndex:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        auto command = m_ArtworksListModel.removeKeywordAt(originalIndex, keywordIndex);
        m_CommandManager.processCommand(command);
    }

    void FilteredArtworksListModel::removeLastKeyword(int proxyIndex) {
        LOG_INFO << "proxyIndex:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        auto command = m_ArtworksListModel.removeLastKeyword(originalIndex);
        m_CommandManager.processCommand(command);
    }

    void FilteredArtworksListModel::appendKeyword(int proxyIndex, const QString &keyword) {
        LOG_INFO << "proxyIndex:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        auto command = m_ArtworksListModel.appendKeyword(originalIndex, keyword);
        m_CommandManager.processCommand(command);
    }

    void FilteredArtworksListModel::pasteKeywords(int proxyIndex, const QStringList &keywords) {
        LOG_INFO << "proxyIndex:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        auto command = m_ArtworksListModel.pasteKeywords(originalIndex, keywords);
        m_CommandManager.processCommand(command);
    }

    void FilteredArtworksListModel::addSuggestedKeywords(int proxyIndex, const QStringList &keywords) {
        LOG_INFO << "proxyIndex:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        auto command = m_ArtworksListModel.addSuggestedKeywords(originalIndex, keywords);
        m_CommandManager.processCommand(command);
    }

    void FilteredArtworksListModel::editKeyword(int proxyIndex, int keywordIndex, const QString &replacement) {
        LOG_INFO << "proxyIndex:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        auto command = m_ArtworksListModel.editKeyword(originalIndex, keywordIndex, replacement);
        m_CommandManager.processCommand(command);
    }

    void FilteredArtworksListModel::plainTextEdit(int proxyIndex, const QString &rawKeywords, bool spaceIsSeparator) {
        LOG_INFO << "proxyIndex:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        auto command = m_ArtworksListModel.plainTextEdit(originalIndex, rawKeywords, spaceIsSeparator);
        m_CommandManager.processCommand(command);
    }

    void FilteredArtworksListModel::expandPreset(int proxyIndex, int keywordIndex, unsigned int presetID) {
        LOG_INFO << "proxyIndex:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        auto command = m_ArtworksListModel.expandPreset(originalIndex, keywordIndex, presetID, m_PresetsManager);
        m_CommandManager.processCommand(command);
    }

    void FilteredArtworksListModel::expandLastAsPreset(int proxyIndex) {
        LOG_INFO << "proxyIndex:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        auto command = m_ArtworksListModel.expandLastAsPreset(originalIndex, m_PresetsManager);
        m_CommandManager.processCommand(command);
    }

    void FilteredArtworksListModel::addPreset(int proxyIndex, unsigned int presetID) {
        LOG_INFO << "proxyIndex:" << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        auto command = m_ArtworksListModel.addPreset(originalIndex, presetID, m_PresetsManager);
        m_CommandManager.processCommand(command);
    }

    void FilteredArtworksListModel::removeMetadataInSelected() const {
        LOG_DEBUG << "#";
        auto selectedIndices = getSelectedOriginalIndices();
        using namespace Common;
        Common::ArtworkEditFlags flags = ArtworkEditFlags::Clear |
                                         ArtworkEditFlags::EditDescription |
                                         ArtworkEditFlags::EditKeywords |
                                         ArtworkEditFlags::EditTitle;

        auto command = m_ArtworksListModel.removeMetadata(
                           Helpers::IndicesRanges(selectedIndices),
                           flags);
        m_CommandManager.processCommand(command);
    }

    void FilteredArtworksListModel::clearKeywords(int proxyIndex) {
        LOG_INFO << "proxyIndex:" << proxyIndex;
        using namespace Common;
        Common::ArtworkEditFlags flags = ArtworkEditFlags::Clear |
                                         ArtworkEditFlags::EditKeywords;

        auto command = m_ArtworksListModel.removeMetadata(
                           Helpers::IndicesRanges({proxyIndex}),
                           flags);
        m_CommandManager.processCommand(command);
    }

    void FilteredArtworksListModel::detachVectorFromSelected() {
        LOG_DEBUG << "#";
        auto indices = getSelectedOriginalIndices();
        m_ArtworksListModel.detachVectorsFromArtworks(Helpers::IndicesRanges(indices));
    }

    void FilteredArtworksListModel::detachVectorFromArtwork(int proxyIndex) {
        LOG_DEBUG << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        m_ArtworksListModel.detachVectorsFromArtworks(Helpers::IndicesRanges(originalIndex, 1));
    }

    void FilteredArtworksListModel::updateFilter() {
        LOG_DEBUG << "#";
        invalidateFilter();
        emit afterInvalidateFilter();
    }

    QString FilteredArtworksListModel::getKeywordsString(int proxyIndex) {
        QString keywords;
        int originalIndex = getOriginalIndex(proxyIndex);
        ArtworksListModel::ArtworkItem artwork;
        if (m_ArtworksListModel.tryGetArtwork(originalIndex, artwork)) {
            keywords = artwork->getKeywordsString();
        }
        return keywords;
    }

    bool FilteredArtworksListModel::hasTitleWordSpellError(int proxyIndex, const QString &word) {
        bool result = false;
        int originalIndex = getOriginalIndex(proxyIndex);
        LOG_INFO << originalIndex << word;
        ArtworksListModel::ArtworkItem artwork;
        if (m_ArtworksListModel.tryGetArtwork(originalIndex, artwork)) {
            auto *keywordsModel = artwork->getBasicModel();
            result = keywordsModel->hasTitleWordSpellError(word);
        }

        return result;
    }

    bool FilteredArtworksListModel::hasDescriptionWordSpellError(int proxyIndex, const QString &word) {
        bool result = false;
        int originalIndex = getOriginalIndex(proxyIndex);
        LOG_INFO << originalIndex << word;
        ArtworksListModel::ArtworkItem artwork;
        if (m_ArtworksListModel.tryGetArtwork(originalIndex, artwork)) {
            auto *keywordsModel = artwork->getBasicModel();
            result = keywordsModel->hasDescriptionWordSpellError(word);
        }

        return result;
    }

    void FilteredArtworksListModel::registerCurrentItem(int proxyIndex) const {
        LOG_INFO << proxyIndex;
        m_ArtworksListModel.setCurrentIndex(getOriginalIndex(proxyIndex));
    }

    void FilteredArtworksListModel::copyToQuickBuffer(int proxyIndex) const {
        LOG_INFO << proxyIndex;
        int originalIndex = getOriginalIndex(proxyIndex);
        ArtworksListModel::ArtworkItem artwork;
        if (m_ArtworksListModel.tryGetArtwork(originalIndex, artwork)) {
            sendMessage(
                        QuickBufferMessage(
                            artwork->getTitle(),
                            artwork->getDescription(),
                            artwork->getKeywords(),
                            false));
        }
    }

    void FilteredArtworksListModel::itemSelectedChanged(bool value) {
        int plus = value ? +1 : -1;

        m_SelectedArtworksCount += plus;
        emit selectedArtworksCountChanged();
    }

    void FilteredArtworksListModel::onSelectedArtworksRemoved(int value) {
        m_SelectedArtworksCount -= value;
        emit selectedArtworksCountChanged();
    }

    void FilteredArtworksListModel::onSettingsUpdated() {
        LOG_DEBUG << "#";
        updateSearchFlags();
        invalidateFilter();
    }

    void FilteredArtworksListModel::setFilteredItemsSelected(bool selected) {
        setFilteredItemsSelectedEx([](Artworks::ArtworkMetadata*) { return true; }, selected, false);
    }

    void FilteredArtworksListModel::setFilteredItemsSelectedEx(const std::function<bool (ArtworkItem const &)> pred,
                                                               bool selected,
                                                               bool unselectAllFirst) {
        LOG_INFO << selected;

        std::vector<int> indices;
        const int size = this->rowCount();
        indices.reserve(size);
        int selectedCount = 0;

        for (int row = 0; row < size; ++row) {
            int index = getOriginalIndex(row);
            ArtworksListModel::ArtworkItem artwork;
            if (!m_ArtworksListModel.tryGetArtwork(index, artwork)) { break; }

            if (unselectAllFirst) {
                artwork->setIsSelected(false);
            }

            if (pred(artwork)) {
                artwork->setIsSelected(selected);
                selectedCount++;
            }

            indices.push_back(index);
        }

        LOG_DEBUG << "Set selected" << selectedCount << "item(s) to" << selected;
        m_ArtworksListModel.updateItems(Helpers::IndicesRanges(indices),
                                        QVector<int>() << ArtworksListModel::IsSelectedRole);
        emit allItemsSelectedChanged();

        m_ArtworksListModel.unsetCurrentIndex();
    }

    Artworks::ArtworksSnapshot FilteredArtworksListModel::getSelectedOriginalItems() const {
        Artworks::ArtworksSnapshot items = filterItems<std::shared_ptr<Artworks::ArtworkMetadata>>(
            [](ArtworkItem const &artwork) { return artwork->isSelected(); },
            [] (ArtworkItem const &artwork, int, int) { return artwork; });

        return items;
    }

    template<typename T>
    std::vector<T> FilteredArtworksListModel::filterItems(std::function<bool (ArtworkItem const &)> pred,
                                                          std::function<T(ArtworkItem const &, int, int)> mapper) const {
        std::vector<T> filteredArtworks;
        const int size = this->rowCount();
        filteredArtworks.reserve(size);

        for (int proxyIndex = 0; proxyIndex < size; ++proxyIndex) {
            int originalIndex = getOriginalIndex(proxyIndex);
            ArtworksListModel::ArtworkItem artwork;
            if (m_ArtworksListModel.tryGetArtwork(originalIndex, artwork)) {
                Q_ASSERT(!artwork->isRemoved());

                if (pred(artwork)) {
                    filteredArtworks.push_back(mapper(artwork, originalIndex, proxyIndex));
                }
            }
        }

        LOG_INFO << "Filtered" << filteredArtworks.size() << "item(s)";

        return filteredArtworks;
    }

    std::vector<int> FilteredArtworksListModel::getSelectedOriginalIndices() const {
        std::vector<int> items = filterItems<int>(
            [](ArtworkItem const &artwork) { return artwork->isSelected(); },
            [] (ArtworkItem const &, int index, int) { return index; });

        return items;
    }

    void FilteredArtworksListModel::forceUnselectAllItems() {
        LOG_DEBUG << "#";
        m_ArtworksListModel.unselectAllItems();
        m_SelectedArtworksCount = 0;
        emit selectedArtworksCountChanged();
        emit allItemsSelectedChanged();
    }

    void FilteredArtworksListModel::updateSearchFlags() {
        bool searchUsingAnd = m_SettingsModel.getSearchUsingAnd();
        bool searchByFilepath = m_SettingsModel.getSearchByFilepath();
        // default search is not case sensitive
        m_SearchFlags = searchUsingAnd ? Common::SearchFlags::AllTermsEverything :
                                    Common::SearchFlags::AnyTermsEverything;
        Common::ApplyFlag(m_SearchFlags, searchByFilepath, Common::SearchFlags::Filepath);
    }

    bool FilteredArtworksListModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        Q_UNUSED(sourceParent);

        ArtworksListModel::ArtworkItem artwork;
        if (!m_ArtworksListModel.tryGetArtwork(sourceRow, artwork)) { return false; }
        if (artwork->isRemoved()) { return false; }

        bool hasMatch = false;

        if (m_ArtworksListModel.isInSelectedDirectory(sourceRow)) {
            hasMatch = true;

            if (!m_SearchTerm.trimmed().isEmpty()) {
                hasMatch = Helpers::hasSearchMatch(m_SearchTerm, artwork, m_SearchFlags);
            }
        }

        return hasMatch;
    }

    bool FilteredArtworksListModel::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const {
        if (!m_SortingEnabled) {
            return QSortFilterProxyModel::lessThan(sourceLeft, sourceRight);
        }

        ArtworksListModel::ArtworkItem leftArtwork, rightArtwork;
        bool result = false;

        if (m_ArtworksListModel.tryGetArtwork(sourceLeft.row(), leftArtwork) &&
                m_ArtworksListModel.tryGetArtwork(sourceRight.row(), rightArtwork)) {
            const QString &leftFilepath = leftArtwork->getFilepath();
            const QString &rightFilepath = rightArtwork->getFilepath();

            QFileInfo leftFI(leftFilepath);
            QFileInfo rightFI(rightFilepath);

            QString leftFilename = leftFI.fileName();
            QString rightFilename = rightFI.fileName();

            int filenamesResult = QString::compare(leftFilename, rightFilename);

            if (filenamesResult == 0) {
                result = QString::compare(leftFilepath, rightFilepath) < 0;
            } else {
                result = filenamesResult < 0;
            }
        }

        return result;
    }
}
