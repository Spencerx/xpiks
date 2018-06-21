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
#include "artworkmetadata.h"
#include "artworksrepository.h"
#include "artworkelement.h"
#include "settingsmodel.h"
#include "../Commands/commandmanager.h"
#include "../Commands/combinededitcommand.h"
#include "../Common/flags.h"
#include "../Helpers/indiceshelper.h"
#include "../Common/defines.h"
#include "../Helpers/filterhelpers.h"
#include "../Models/previewartworkelement.h"
#include "../QuickBuffer/quickbuffer.h"
#include "videoartwork.h"

namespace Models {
    FilteredArtworksListModel::FilteredArtworksListModel(ArtworksListModel &artworksListModel, QObject *parent):
        QSortFilterProxyModel(parent),
        m_ArtworksListModel(artworksListModel),
        m_SelectedArtworksCount(0),
        m_SortingEnabled(false) {
        // m_SortingEnabled = true;
        // this->sort(0);
        m_SearchFlags = Common::SearchFlags::AnyTermsEverything;
    }

    void FilteredArtworksListModel::updateSearchFlags() {
        SettingsModel *settingsModel = m_CommandManager->getSettingsModel();
        bool searchUsingAnd = settingsModel->getSearchUsingAnd();
        bool searchByFilepath = settingsModel->getSearchByFilepath();
        // default search is not case sensitive
        m_SearchFlags = searchUsingAnd ? Common::SearchFlags::AllTermsEverything :
                                    Common::SearchFlags::AnyTermsEverything;
        Common::ApplyFlag(m_SearchFlags, searchByFilepath, Common::SearchFlags::Filepath);
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

    void FilteredArtworksListModel::spellCheckAllItems() {
        LOG_DEBUG << "#";
        auto allArtworks = getAllOriginalItems();
        xpiks()->submitForSpellCheck(allArtworks);
        xpiks()->reportUserAction(Connectivity::UserAction::SpellCheck);
    }

    int FilteredArtworksListModel::getOriginalIndex(int index) const {
        QModelIndex originalIndex = mapToSource(this->index(index, 0));
        int row = originalIndex.row();

        return row;
    }

    int FilteredArtworksListModel::getDerivedIndex(int originalIndex) const {
        ArtItemsModel *artItemsModel = getArtItemsModel();
        QModelIndex index = mapFromSource(artItemsModel->index(originalIndex, 0));
        int row = index.row();

        return row;
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

    void FilteredArtworksListModel::removeSelectedArtworks() {
        LOG_DEBUG << "#";
        //m_ArtworksListModel.removeFiles()
    }

    void FilteredArtworksListModel::updateSelectedArtworks() {
        LOG_DEBUG << "#";
        QVector<int> indices = getSelectedOriginalIndices();
        m_ArtworksListModel.updateItems(Helpers::IndicesRanges(indices), roles);
    }

    void FilteredArtworksListModel::saveSelectedArtworks(bool overwriteAll, bool useBackups) {
        LOG_INFO << "ovewriteAll:" << overwriteAll << "useBackups:" << useBackups;
        // former patchSelectedArtworks
        auto itemsToSave = getFilteredOriginalItems<ArtworkMetadata*>(
                    [&overwriteAll](ArtworkMetadata *artwork) {
                return artwork->isSelected() && !artwork->isReadOnly() && (artwork->isModified() || overwriteAll);
    },
                [] (ArtworkMetadata *artwork, int, int) { return artwork; });

        xpiks()->writeMetadata(itemsToSave, useBackups);
    }

    void FilteredArtworksListModel::wipeMetadataFromSelectedArtworks(bool useBackups) {
        LOG_INFO << "useBackups:" << useBackups;

        auto selectedArtworks = getSelectedArtworksSnapshot();
        Artworks::ArtworksSnapshot snapshot(selectedArtworks);
        xpiks()->wipeAllMetadata(snapshot, useBackups);
    }

    void FilteredArtworksListModel::setSelectedForUpload() {
        LOG_DEBUG << "#";
        auto selectedArtworks = getSelectedArtworksSnapshot();
        Artworks::ArtworksSnapshot snapshot(selectedArtworks);
        xpiks()->setArtworksForUpload(snapshot);
    }

    void FilteredArtworksListModel::setSelectedForZipping() {
        LOG_DEBUG << "#";
        auto itemsForZipping = getFilteredOriginalItems<ArtworkMetadata *>(
                    [](ArtworkMetadata *artwork) {
                if (!artwork->isSelected()) { return false; }
                ImageArtwork *image = dynamic_cast<ImageArtwork*>(artwork);
                return (image != nullptr) && (image->hasVectorAttached());
            },
                [] (ArtworkMetadata *artwork, int, int) { return artwork; });

        Artworks::ArtworksSnapshot snapshot(itemsForZipping);
        xpiks()->setArtworksForZipping(snapshot);
    }

    bool FilteredArtworksListModel::areSelectedArtworksSaved() {
        auto selectedArtworks = getSelectedOriginalItems();
        bool anyModified = false;

        for (ArtworkMetadata *artwork: selectedArtworks) {
            if (artwork->isModified()) {
                anyModified = true;
                break;
            }
        }

        LOG_DEBUG << "any modified:" << anyModified;

        return !anyModified;
    }

    void FilteredArtworksListModel::spellCheckSelected() {
        LOG_DEBUG << "#";
        auto selectedArtworks = getSelectedOriginalItems();
        xpiks()->submitForSpellCheck(selectedArtworks);
        xpiks()->reportUserAction(Connectivity::UserAction::SpellCheck);
    }

    int FilteredArtworksListModel::getModifiedSelectedCount(bool overwriteAll) {
        auto selectedArtworks = getSelectedOriginalItems();
        int modifiedCount = 0;

        for (ArtworkMetadata *artwork: selectedArtworks) {
            if (!artwork->isReadOnly() && (artwork->isModified() || overwriteAll)) {
                modifiedCount++;
            }
        }

        return modifiedCount;
    }

    void FilteredArtworksListModel::removeArtworksDirectory(int index) {
        LOG_DEBUG << "#";
        ArtItemsModel *artItemsModel = getArtItemsModel();

        artItemsModel->removeArtworksDirectory(index);
        emit selectedArtworksCountChanged();
    }

    void FilteredArtworksListModel::deleteKeywordsFromSelected() {
        auto selectedItems = getSelectedOriginalItems();
        xpiks()->deleteKeywordsFromArtworks(selectedItems);
    }

    void FilteredArtworksListModel::setSelectedForCsvExport() {
        LOG_DEBUG << "#";
        auto selectedArtworks = getSelectedArtworksSnapshot();
        xpiks()->setArtworksForCsvExport(selectedArtworks);
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
            this->setFilteredItemsSelectedEx([](ArtworkMetadata *artwork) {
                return artwork->isModified();
            }, isSelected, unselectFirst);
            break;
        }
        case 3: {
            // select Images
            this->setFilteredItemsSelectedEx([](ArtworkMetadata *artwork) {
                ImageArtwork *image = dynamic_cast<ImageArtwork*>(artwork);
                return (image != nullptr) ? !image->hasVectorAttached() : false;
            }, isSelected, unselectFirst);
            break;
        }
        case 4: {
            // select Vectors
            this->setFilteredItemsSelectedEx([](ArtworkMetadata *artwork) {
                ImageArtwork *image = dynamic_cast<ImageArtwork*>(artwork);
                return (image != nullptr) ? image->hasVectorAttached() : false;
            }, isSelected, unselectFirst);
            break;
        }
        case 5: {
            // select Videos
            this->setFilteredItemsSelectedEx([](ArtworkMetadata *artwork) {
                return dynamic_cast<VideoArtwork*>(artwork) != nullptr;
            }, isSelected, unselectFirst);
            break;
        }
        default:
            break;
        }
    }

    void FilteredArtworksListModel::reimportMetadataForSelected() {
        LOG_DEBUG << "#";
        auto selectedArtworks = getSelectedOriginalItems();

        for (auto *artwork: selectedArtworks) {
            artwork->prepareForReimport();
        }

        int importID = xpiks()->reimportMetadata(selectedArtworks);
        ArtItemsModel *artItemsModel = getArtItemsModel();
        artItemsModel->raiseArtworksReimported(importID, (int)selectedArtworks.size());
    }

    int FilteredArtworksListModel::findSelectedItemIndex() const {
        int index = -1;

        QVector<int> indices = getSelectedIndices();
        if (indices.length() == 1) {
            index = indices.first();
        }

        return index;
    }

    void FilteredArtworksListModel::removeMetadataInSelected() const {
        LOG_DEBUG << "#";
        auto selectedArtworks = getSelectedArtworksSnapshot();
        Common::CombinedEditFlags flags = Common::CombinedEditFlags::None;
        using namespace Common;
        Common::SetFlag(flags, CombinedEditFlags::EditDescription);
        Common::SetFlag(flags, CombinedEditFlags::EditKeywords);
        Common::SetFlag(flags, CombinedEditFlags::EditTitle);
        Common::SetFlag(flags, CombinedEditFlags::Clear);
        removeMetadataInItems(selectedArtworks, flags);
    }

    void FilteredArtworksListModel::clearKeywords(int index) {
        LOG_INFO << "index:" << index;
        ArtItemsModel *artItemsModel = getArtItemsModel();
        int originalIndex = getOriginalIndex(index);
        ArtworkMetadata *metadata = artItemsModel->getArtwork(originalIndex);

        if ((metadata != NULL) && (!metadata->areKeywordsEmpty())) {
            removeKeywordsInItem(metadata);
        }
    }

    void FilteredArtworksListModel::focusNextItem(int index) {
        LOG_INFO << "index:" << index;
        if (0 <= index && index < rowCount() - 1) {
            QModelIndex nextQIndex = this->index(index + 1, 0);
            QModelIndex sourceIndex = mapToSource(nextQIndex);
            ArtItemsModel *artItemsModel = getArtItemsModel();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(sourceIndex.row());

            if (metadata != NULL) {
                metadata->requestFocus(+1);
            }
        }
    }

    void FilteredArtworksListModel::focusPreviousItem(int index) {
        LOG_INFO << "index:" << index;
        if (0 < index && index < rowCount()) {
            QModelIndex nextQIndex = this->index(index - 1, 0);
            QModelIndex sourceIndex = mapToSource(nextQIndex);
            ArtItemsModel *artItemsModel = getArtItemsModel();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(sourceIndex.row());

            if (metadata != NULL) {
                metadata->requestFocus(-1);
            }
        }
    }

    void FilteredArtworksListModel::focusCurrentItemKeywords(int index) {
        LOG_INFO << "index:" << index;
        if ((0 <= index) && (index < rowCount())) {
            QModelIndex qIndex = this->index(index, 0);
            QModelIndex sourceIndex = mapToSource(qIndex);
            ArtItemsModel *artItemsModel = getArtItemsModel();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(sourceIndex.row());

            if (metadata != NULL) {
                metadata->requestFocus(-1);
            }
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

        ArtItemsModel *artItemsModel = getArtItemsModel();
        artItemsModel->updateAllItems();
    }

    void FilteredArtworksListModel::detachVectorFromSelected() {
        LOG_DEBUG << "#";
        QVector<int> indices = getSelectedOriginalIndices();
        ArtItemsModel *artItemsModel = getArtItemsModel();
        artItemsModel->detachVectorsFromArtworks(indices);
    }

    void FilteredArtworksListModel::detachVectorFromArtwork(int index) {
        LOG_DEBUG << index;
        int originalIndex = getOriginalIndex(index);
        ArtItemsModel *artItemsModel = getArtItemsModel();
        artItemsModel->detachVectorsFromArtworks(QVector<int>() << originalIndex);
    }

    QObject *FilteredArtworksListModel::getArtworkMetadata(int index) {
        int originalIndex = getOriginalIndex(index);
        ArtItemsModel *artItemsModel = getArtItemsModel();
        QObject *item = artItemsModel->getArtworkMetadata(originalIndex);

        return item;
    }

    QObject *FilteredArtworksListModel::getBasicModel(int index) {
        int originalIndex = getOriginalIndex(index);
        ArtItemsModel *artItemsModel = getArtItemsModel();
        QObject *item = artItemsModel->getBasicModel(originalIndex);

        return item;
    }

    QString FilteredArtworksListModel::getKeywordsString(int index) {
        int originalIndex = getOriginalIndex(index);
        ArtItemsModel *artItemsModel = getArtItemsModel();
        ArtworkMetadata *artwork = artItemsModel->getArtwork(originalIndex);
        QString keywords;
        if (artwork != nullptr) {
            keywords = artwork->getKeywordsString();
        }
        return keywords;
    }

    bool FilteredArtworksListModel::hasTitleWordSpellError(int index, const QString &word) {
        bool result = false;

        if (0 <= index && index < rowCount()) {
            int originalIndex = getOriginalIndex(index);
            LOG_INFO << originalIndex << word;
            ArtItemsModel *artItemsModel = getArtItemsModel();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(originalIndex);
            if (metadata != NULL) {
                auto *keywordsModel = metadata->getBasicModel();
                result = keywordsModel->hasTitleWordSpellError(word);
            }
        }

        return result;
    }

    bool FilteredArtworksListModel::hasDescriptionWordSpellError(int index, const QString &word) {
        bool result = false;

        if (0 <= index && index < rowCount()) {
            int originalIndex = getOriginalIndex(index);
            LOG_INFO << originalIndex << word;
            ArtItemsModel *artItemsModel = getArtItemsModel();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(originalIndex);
            if (metadata != NULL) {
                auto *keywordsModel = metadata->getBasicModel();
                result = keywordsModel->hasDescriptionWordSpellError(word);
            }
        }

        return result;
    }

    void FilteredArtworksListModel::registerCurrentItem(int index) const {
        LOG_INFO << index;

        if (0 <= index && index < rowCount()) {
            int originalIndex = getOriginalIndex(index);
            ArtItemsModel *artItemsModel = getArtItemsModel();
            ArtworkMetadata *artwork = artItemsModel->getArtwork(originalIndex);

            if (artwork != NULL) {
                xpiks()->registerCurrentItem(artwork);
            }
        }
    }

    void FilteredArtworksListModel::copyToQuickBuffer(int index) const {
        LOG_INFO << index;

        if (0 <= index && index < rowCount()) {
            int originalIndex = getOriginalIndex(index);
            ArtItemsModel *artItemsModel = getArtItemsModel();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(originalIndex);

            if (metadata != NULL) {
                auto *basicModel = metadata->getBasicModel();
                auto *quickBuffer = m_CommandManager->getQuickBuffer();
                quickBuffer->setFromBasicModel(basicModel);
            }
        }
    }

    void FilteredArtworksListModel::fillFromQuickBuffer(int index) const {
        LOG_INFO << index;

        if (0 <= index && index < rowCount()) {
            int originalIndex = getOriginalIndex(index);
            ArtItemsModel *artItemsModel = getArtItemsModel();
            artItemsModel->fillFromQuickBuffer(originalIndex);
        }
    }

    void FilteredArtworksListModel::suggestCorrectionsForSelected() const {
        using namespace Common;
        SuggestionFlags flags = SuggestionFlags::None;
        Common::SetFlag(flags, SuggestionFlags::Description);
        Common::SetFlag(flags, SuggestionFlags::Title);
        Common::SetFlag(flags, SuggestionFlags::Keywords);

        auto itemsForSuggestions = getFilteredOriginalItems<std::pair<Common::IMetadataOperator *, int> >(
                    [](ArtworkMetadata *artwork) { return artwork->isSelected(); },
                    [] (ArtworkMetadata *metadata, int index, int) { return std::pair<Common::IMetadataOperator *, int>(metadata, index); });
        xpiks()->setupSpellCheckSuggestions(itemsForSuggestions, flags);
    }

    void FilteredArtworksListModel::generateCompletions(const QString &prefix, int index) {
        if (0 <= index && index < rowCount()) {
            int originalIndex = getOriginalIndex(index);

            ArtItemsModel *artItemsModel = getArtItemsModel();
            auto *basicModel = artItemsModel->getBasicModel(originalIndex);

            xpiks()->generateCompletions(prefix, basicModel);
        }
    }

    void FilteredArtworksListModel::reviewDuplicatesInSelected() const {
        auto itemsForSuggestions = getFilteredOriginalItems<ArtworkMetadata *>(
                    [](ArtworkMetadata *artwork) { return artwork->hasDuplicates(); },
                    [] (ArtworkMetadata *artwork, int, int) { return artwork; });
        xpiks()->setupDuplicatesModel(itemsForSuggestions);
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

    void FilteredArtworksListModel::onSpellCheckerAvailable(bool afterRestart) {
        LOG_INFO << "after restart:" << afterRestart;
        if (afterRestart) {
            this->spellCheckAllItems();
        }
    }

    void FilteredArtworksListModel::onSettingsUpdated() {
        LOG_DEBUG << "#";
        updateSearchFlags();
        invalidateFilter();
    }

    void FilteredArtworksListModel::removeMetadataInItems(Artworks::ArtworksSnapshot::Container &itemsToClear, Common::CombinedEditFlags flags) const {
        LOG_INFO << itemsToClear.size() << "item(s) with flags =" << (int)flags;
        std::shared_ptr<Commands::CombinedEditCommand> combinedEditCommand(new Commands::CombinedEditCommand(
                flags,
                itemsToClear));

        m_CommandManager->processCommand(combinedEditCommand);
    }

    void FilteredArtworksListModel::removeKeywordsInItem(ArtworkMetadata *artwork) {
        Common::CombinedEditFlags flags = Common::CombinedEditFlags::None;
        Common::SetFlag(flags, Common::CombinedEditFlags::EditKeywords);
        Common::SetFlag(flags, Common::CombinedEditFlags::Clear);

        Artworks::ArtworksSnapshot::Container items;

        items.emplace_back(new ArtworkMetadataLocker(artwork));

        removeMetadataInItems(items, flags);
    }

    void FilteredArtworksListModel::setFilteredItemsSelected(bool selected) {
        setFilteredItemsSelectedEx([](ArtworkMetadata*) { return true; }, selected, false);
    }

    void FilteredArtworksListModel::setFilteredItemsSelectedEx(const std::function<bool (ArtworkMetadata *)> pred, bool selected, bool unselectFirst) {
        LOG_INFO << selected;
        ArtItemsModel *artItemsModel = getArtItemsModel();

        QVector<int> indices;
        int size = this->rowCount();
        indices.reserve(size);
        int selectedCount = 0;

        for (int row = 0; row < size; ++row) {
            QModelIndex proxyIndex = this->index(row, 0);
            QModelIndex originalIndex = this->mapToSource(proxyIndex);

            int index = originalIndex.row();
            ArtworkMetadata *artwork = artItemsModel->getArtwork(index);
            if (artwork != NULL) {
                if (unselectFirst) {
                    artwork->setIsSelected(false);
                }

                if (pred(artwork)) {
                    artwork->setIsSelected(selected);
                    selectedCount++;
                }

                indices << index;
            }
        }

        LOG_DEBUG << "Set selected" << selectedCount << "item(s) to" << selected;
        artItemsModel->updateItems(indices, QVector<int>() << ArtItemsModel::IsSelectedRole);
        emit allItemsSelectedChanged();

        xpiks()->clearCurrentItem();
    }

    void FilteredArtworksListModel::invertFilteredItemsSelected() {
        LOG_DEBUG << "#";
        ArtItemsModel *artItemsModel = getArtItemsModel();
        QVector<int> indices;
        int size = this->rowCount();
        indices.reserve(size);

        for (int row = 0; row < size; ++row) {
            QModelIndex proxyIndex = this->index(row, 0);
            QModelIndex originalIndex = this->mapToSource(proxyIndex);

            int index = originalIndex.row();
            ArtworkMetadata *metadata = artItemsModel->getArtwork(index);
            if (metadata != NULL) {
                metadata->invertSelection();
                indices << index;
            }
        }

        artItemsModel->updateItems(indices, QVector<int>() << ArtItemsModel::IsSelectedRole);
        emit allItemsSelectedChanged();
    }

    Artworks::WeakArtworksSnapshot FilteredArtworksListModel::getSelectedOriginalItems() const {
        Artworks::WeakArtworksSnapshot items = getFilteredOriginalItems<ArtworkMetadata *>(
            [](ArtworkMetadata *metadata) { return metadata->isSelected(); },
            [] (ArtworkMetadata *metadata, int, int) { return metadata; });

        return items;
    }

    Artworks::ArtworksSnapshot::Container FilteredArtworksListModel::getSelectedArtworksSnapshot() const {
        return getFilteredOriginalItems<std::shared_ptr<ArtworkMetadataLocker> >(
            [](ArtworkMetadata *metadata) { return metadata->isSelected(); },
            [] (ArtworkMetadata *metadata, int, int) {
            return std::shared_ptr<ArtworkMetadataLocker>(new ArtworkMetadataLocker(metadata));
    });
}

    template<typename T>
    std::vector<T> FilteredArtworksListModel::getFilteredOriginalItems(std::function<bool (ArtworkMetadata *)> pred,
                                                                        std::function<T(ArtworkMetadata *, int, int)> mapper) const {
        ArtItemsModel *artItemsModel = getArtItemsModel();

        std::vector<T> filteredArtworks;
        int size = this->rowCount();

        filteredArtworks.reserve(size);

        for (int row = 0; row < size; ++row) {
            QModelIndex proxyIndex = this->index(row, 0);
            QModelIndex originalIndex = this->mapToSource(proxyIndex);

            int index = originalIndex.row();
            ArtworkMetadata *artwork = artItemsModel->getArtwork(index);

            if (artwork != NULL &&
                    !artwork->isRemoved() &&
                    pred(artwork)) {
                filteredArtworks.push_back(mapper(artwork, index, row));
            }
        }

        LOG_INFO << "Filtered" << filteredArtworks.size() << "item(s)";

        return filteredArtworks;
    }

    Artworks::WeakArtworksSnapshot FilteredArtworksListModel::getAllOriginalItems() const {
        Artworks::WeakArtworksSnapshot items = getFilteredOriginalItems<ArtworkMetadata *>(
            [](ArtworkMetadata *) { return true; },
            [] (ArtworkMetadata *artwork, int, int) { return artwork; });

        return items;
    }

    std::vector<int> FilteredArtworksListModel::getSelectedOriginalIndices() const {
        std::vector<int> items = getFilteredOriginalItems<int>(
            [](ArtworkMetadata *artwork) { return artwork->isSelected(); },
            [] (ArtworkMetadata *, int index, int) { return index; });

        return items;
    }

    std::vector<int> FilteredArtworksListModel::getSelectedIndices() const {
        std::vector<int> items = getFilteredOriginalItems<int>(
            [](ArtworkMetadata *artwork) { return artwork->isSelected(); },
            [] (ArtworkMetadata *, int, int originalIndex) { return originalIndex; });

        return items;
    }

    void FilteredArtworksListModel::forceUnselectAllItems() {
        LOG_DEBUG << "#";
        ArtItemsModel *artItemsModel = getArtItemsModel();
        artItemsModel->forceUnselectAllItems();
        m_SelectedArtworksCount = 0;
        emit selectedArtworksCountChanged();
        emit allItemsSelectedChanged();
        xpiks()->clearCurrentItem();
    }

    bool FilteredArtworksListModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        Q_UNUSED(sourceParent);

        ArtItemsModel *artItemsModel = getArtItemsModel();
        ArtworkMetadata *artwork = artItemsModel->getArtwork(sourceRow);

        bool hasMatch = false;
        if (artwork == nullptr) { return false; }
        if (artwork->isRemoved()) { return false; }

        ArtworksRepository *repository = m_CommandManager->getArtworksRepository();
        Q_ASSERT(repository != NULL);
        qint64 directoryID = artwork->getDirectoryID();

        bool directoryIsIncluded = repository->isDirectorySelected(directoryID);
        if (directoryIsIncluded) {
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

        ArtItemsModel *artItemsModel = getArtItemsModel();

        ArtworkMetadata *leftMetadata = artItemsModel->getArtwork(sourceLeft.row());
        ArtworkMetadata *rightMetadata = artItemsModel->getArtwork(sourceRight.row());

        bool result = false;

        if (leftMetadata != NULL && rightMetadata != NULL) {
            const QString &leftFilepath = leftMetadata->getFilepath();
            const QString &rightFilepath = rightMetadata->getFilepath();

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

#ifdef CORE_TESTS
    int FilteredArtItemsProxyModel::retrieveNumberOfSelectedItems() {
        return getSelectedOriginalIndices().size();
    }
#endif

    Artworks::ArtworksSnapshot::Container FilteredArtworksListModel::getSearchablePreviewOriginalItems(const QString &searchTerm,
                                                                                                          Common::SearchFlags flags) const {
        return getFilteredOriginalItems<std::shared_ptr<ArtworkMetadataLocker> >(
            [&searchTerm, flags](ArtworkMetadata *artwork) {
            return Helpers::hasSearchMatch(searchTerm, artwork, flags);
        },
            [] (ArtworkMetadata *artwork, int, int) {
            return std::shared_ptr<ArtworkMetadataLocker>(new PreviewArtworkElement(artwork)); });
    }
}
