/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spellchecksuggestionmodel.h"
#include <QQmlEngine>
#include <QHash>
#include <QString>
#include "spellsuggestionsitem.h"
#include "spellcheckservice.h"
#include <Common/flags.h>
#include <Common/logging.h>
#include <Artworks/artworkmetadata.h>
#include <Artworks/iselectedartworkssource.h>
#include <Artworks/basickeywordsmodel.h>
#include <Services/artworksupdatehub.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Helpers/indicesranges.h>
#include <Helpers/cpphelpers.h>

namespace SpellCheck {
    std::vector<std::shared_ptr<SpellSuggestionsItem> > combineSuggestionRequests(const SuggestionsVector &items) {
        QHash<QString, SuggestionsVector > dict;

        size_t size = items.size();
        for (size_t i = 0; i < size; ++i) {
            auto &item = items.at(i);
            const QString &word = item->getWord();
            if (!dict.contains(word)) {
                dict.insert(word, SuggestionsVector());
            }

            dict[word].emplace_back(item);
        }

        SuggestionsVector result;
        result.reserve(size);

        QHash<QString, SuggestionsVector >::iterator i = dict.begin();
        QHash<QString, SuggestionsVector >::iterator end = dict.end();
        for (; i != end; ++i) {
            SuggestionsVector &vector = i.value();

            if (vector.size() > 1) {
                result.emplace_back(std::make_shared<CombinedSpellSuggestions>(i.key(), vector));
            } else {
                result.emplace_back(vector.front());
            }
        }

        return result;
    }

    QHash<Artworks::IMetadataOperator *, KeywordsSuggestionsVector> combineFailedReplacements(const SuggestionsVector &failedReplacements) {
        QHash<Artworks::IMetadataOperator *, KeywordsSuggestionsVector > candidatesToRemove;
        size_t size = failedReplacements.size();
        candidatesToRemove.reserve((int)size);

        for (size_t i = 0; i < size; ++i) {
            auto &item = failedReplacements.at(i);
            std::shared_ptr<KeywordSpellSuggestions> keywordsItem = std::dynamic_pointer_cast<KeywordSpellSuggestions>(item);

            if (keywordsItem) {
                auto *item = keywordsItem->getMetadataOperator();

                if (keywordsItem->isPotentialDuplicate()) {
                    if (!candidatesToRemove.contains(item)) {
                        candidatesToRemove.insert(item, KeywordsSuggestionsVector());
                    }

                    candidatesToRemove[item].emplace_back(keywordsItem);
                }
            } else {
                std::shared_ptr<CombinedSpellSuggestions> combinedItem = std::dynamic_pointer_cast<CombinedSpellSuggestions>(item);
                if (combinedItem) {
                    auto keywordsItems = combinedItem->getKeywordsDuplicateSuggestions();

                    for (auto &keywordsCombinedItem: keywordsItems) {
                        auto *item = keywordsCombinedItem->getMetadataOperator();

                        if (!candidatesToRemove.contains(item)) {
                            candidatesToRemove.insert(item, KeywordsSuggestionsVector());
                        }

                        candidatesToRemove[item].emplace_back(keywordsCombinedItem);
                    }
                } else {
                    LOG_WARNING << "Unsupported failed suggestion type";
                }
            }
        }

        return candidatesToRemove;
    }

    void addMisspelledKeywords(const QStringList &keywords, SuggestionsVector &requests) {
        requests.reserve(requests.size() + keywords.size(););
        for (auto &keywordItem: keywords) {
            if (!keywordItem.isPartOfAKeyword()) {
                requests.emplace_back(
                            std::make_shared<KeywordSpellSuggestions>(
                                keywordItem.m_Word, keywordItem.m_Index, *item));
            } else {
                requests.emplace_back(
                            std::make_shared<KeywordSpellSuggestions>(
                                keywordItem.m_Word, keywordItem.m_Index, keywordItem.m_OriginKeyword, *item));
            }
        }

        LOG_DEBUG << keywords.size() << "keywords requests";
    }

    void addMisspelledTitle(const QStringList &words, SuggestionsVector &requests) {
        requests.reserve(requests.size() + words.size());

        for (auto &word: words) {
            requests.emplace_back(std::make_shared<TitleSpellSuggestions>(word, *item));
        }

        LOG_DEBUG << words.size() << "title requests";
    }

    void addMisspelledDescription(const QStringList &words, SuggestionsVector &request) {
        requests.reserve(requests.size() + words.size());

        for (auto &word: words) {
            requests.emplace_back(std::make_shared<DescriptionSpellSuggestions>(word, *item));
        }

        LOG_DEBUG << words.size() << "description requests";
    }

    SuggestionsVector createSuggestionsRequests(const Artworks::ArtworksSnapshot &snapshot) {
        SuggestionsVector requests;

        for (auto &locker: snapshot.getRawData()) {
            auto *item = locker->getArtworkMetadata();

            auto words = item->retrieveMisspelledKeywords();
            addMisspelledKeywords(words, requests);

            words = item->retrieveMisspelledTitleWords();
            addMisspelledTitle(words, requests);

            words = item->retrieveMisspelledDescriptionWords();
            addMisspelledDescription(words, requests);
        }

        return requests;
    }

    SuggestionsVector createSuggestionsRequests(const Artworks::BasicMetadataModel *basicModel) {
        SuggestionsVector requests;

        auto words = basicModel->retrieveMisspelledKeywords();
        addMisspelledKeywords(words, requests);

        words = basicModel->retrieveMisspelledTitleWords();
        addMisspelledTitle(words, requests);

        words = basicModel->retrieveMisspelledDescriptionWords();
        addMisspelledDescription(words, requests);

        return requests;
    }

    SpellCheckSuggestionModel::SpellCheckSuggestionModel(SpellCheckService &spellCheckerService,
                                                         Artworks::ISelectedArtworksSource &artworksSource,
                                                         Services::ArtworksUpdateHub &artworksUpdateHub):
        QAbstractListModel(),
        m_SpellCheckerService(spellCheckerService),
        m_ArtworksSource(artworksSource),
        m_ArtworksListModel(artworksUpdateHub)
    {
    }

    int SpellCheckSuggestionModel::getArtworksCount() const {
         return m_CheckedItems.empty() ? 1 : (int)m_CheckedItems.size();
    }

    bool SpellCheckSuggestionModel::getAnythingSelected() const {
        bool anySelected = false;

        for (auto &item: m_SuggestionsList) {
            if (item->anyReplacementSelected()) {
                anySelected = true;
                break;
            }
        }

        return anySelected;
    }

    QObject *SpellCheckSuggestionModel::getSuggestionItself(int index) const {
        SpellSuggestionsItem *item = NULL;

        if (0 <= index && index < (int)m_SuggestionsList.size()) {
            item = m_SuggestionsList.at(index).get();
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        }

        return item;
    }

    void SpellCheckSuggestionModel::clearModel() {
        beginResetModel();
        {
            m_SuggestionsList.clear();
            m_CheckedItems.clear();
        }
        endResetModel();
        emit artworksCountChanged();
    }

    void SpellCheckSuggestionModel::submitCorrections() const {
        LOG_DEBUG << "#";
        bool anyChanged = false;

        SuggestionsVector failedItems;

        for (auto &item: m_SuggestionsList) {
            if (item->anyReplacementSelected()) {
                item->replaceToSuggested();

                if (!item->getReplacementSucceeded()) {
                    failedItems.push_back(item);
                } else {
                    anyChanged = true;
                }
            }
        }

        if (processFailedReplacements(failedItems)) {
            anyChanged = true;
        }

        if (anyChanged) {
            for (auto &item: m_SuggestionsList) {
                item->finalizeReplacement();
            }

            m_SpellCheckerService.submitArtworks(m_CheckedItems);
        }

        m_ArtworksUpdateHub.updateArtworks(m_CheckedItems);
    }

    void SpellCheckSuggestionModel::resetAllSuggestions() {
        LOG_DEBUG << "#";
        for (auto &item: m_SuggestionsList) {
            item->setReplacementIndex(-1);
        }
    }

    void SpellCheckSuggestionModel::setupItem(Artworks::BasicMetadataModel *item) {
        Q_ASSERT(item != NULL);
        LOG_DEBUG << "#";
        auto requests = createSuggestionsRequests(item);
        setupRequests(requests);
    }

    void SpellCheckSuggestionModel::setupArtworks(Artworks::ArtworksSnapshot &snapshot) {
        LOG_INFO << snapshot.size() << "item(s)";
        auto requests = createSuggestionsRequests(snapshot);
        setupRequests(requests);
        m_CheckedItems = std::move(snapshot);
    }

    void SpellCheckSuggestionModel::setArtworks(const Artworks::ArtworksSnapshot &snapshot) {
        this->setupItems(
                    Helpers::map<Artworks::ArtworksSnapshot::ItemType, Artworks::BasicMetadataModel*>(
                        snapshot.getRawData(),
                        [](const std::shared_ptr<Artworks::ArtworkMetadataLocker> &locker) {
                        return locker->getArtworkMetadata()->getBasicModel();
                    }),
                Common::SuggestionFlags::All);
    }

    bool SpellCheckSuggestionModel::processFailedReplacements(const SuggestionsVector &failedReplacements) const {
        LOG_INFO << failedReplacements.size() << "failed items";

        auto candidatesToRemove = combineFailedReplacements(failedReplacements);

        auto it = candidatesToRemove.begin();
        auto itEnd = candidatesToRemove.end();

        bool anyReplaced = false;
        for (; it != itEnd; ++it) {
            auto *item = it.key();

            if (item->processFailedKeywordReplacements(it.value())) {
                anyReplaced = true;
            }
        }

        return anyReplaced;
    }

    SuggestionsVector SpellCheckSuggestionModel::setupSuggestions(const SuggestionsVector &items) {
        LOG_INFO << items.size() << "item(s)";
        // another vector for requests with available suggestions
        SuggestionsVector executedRequests;
        executedRequests.reserve(items.size());

        for (auto &item: items) {
            QStringList suggestions = m_SpellCheckerService.suggestCorrections(item->getWord());
            if (!suggestions.isEmpty()) {
                item->setSuggestions(suggestions);
                executedRequests.push_back(item);
            }
        }

        return executedRequests;
    }

    void SpellCheckSuggestionModel::setupRequests(const SuggestionsVector &requests) {
        auto combinedRequests = combineSuggestionRequests(requests);
        LOG_INFO << combinedRequests.size() << "combined request(s)";

        auto executedRequests = setupSuggestions(combinedRequests);
        LOG_INFO << executedRequests.size() << "executed request(s)";

#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
        for (auto &executedItem: executedRequests) {
            LOG_INFO << executedItem->toDebugString();
        }
#endif

        beginResetModel();
        {
            m_SuggestionsList.clear();
            m_SuggestionsList = executedRequests;
        }
        endResetModel();

        emit artworksCountChanged();
    }

    int SpellCheckSuggestionModel::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return (int)m_SuggestionsList.size();
    }

    QVariant SpellCheckSuggestionModel::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if (row < 0 || row >= (int)m_SuggestionsList.size()) { return QVariant(); }

        auto &item = m_SuggestionsList.at(row);

        switch (role) {
        case WordRole:
            return item->getWord();
        case ReplacementIndexRole:
            return item->getReplacementIndex();
        case ReplacementOriginRole:
            return item->getReplacementOrigin();
        default:
            return QVariant();
        }
    }

    QHash<int, QByteArray> SpellCheckSuggestionModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[WordRole] = "word";
        roles[ReplacementIndexRole] = "replacementindex";
        roles[ReplacementOriginRole] = "replacementorigin";
        return roles;
    }
}
