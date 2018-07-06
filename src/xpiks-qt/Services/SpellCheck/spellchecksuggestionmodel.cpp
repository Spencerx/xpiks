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
#include <Artworks/basickeywordsmodel.h>
#include <Commands/appmessages.h>
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

    SuggestionsVector createSuggestionsRequests(Common::SuggestionFlags flags,
                                                const std::vector<Artworks::IMetadataOperator *> &items) {
        SuggestionsVector requests;

        using namespace Common;

        if (Common::HasFlag(flags, SuggestionFlags::Keywords)) {
            for (auto *item: items) {
                auto misspelledKeywords = item->retrieveMisspelledKeywords();
                requests.reserve(requests.size() + misspelledKeywords.size());
                for (auto &keywordItem: misspelledKeywords) {
                    if (!keywordItem.isPartOfAKeyword()) {
                        requests.emplace_back(
                                    std::make_shared<KeywordSpellSuggestions>(
                                        keywordItem.m_Word, keywordItem.m_Index, item));
                    } else {
                        requests.emplace_back(
                                    std::make_shared<KeywordSpellSuggestions>(
                                        keywordItem.m_Word, keywordItem.m_Index, keywordItem.m_OriginKeyword, item));
                    }
                }
                LOG_DEBUG << misspelledKeywords.size() << "keywords requests";
            }
        }

        if (Common::HasFlag(flags, SuggestionFlags::Title)) {
            for (auto *item: items) {
                QStringList misspelledWords = item->retrieveMisspelledTitleWords();
                requests.reserve(requests.size() + misspelledWords.size());

                for (auto &word: misspelledWords) {
                    requests.emplace_back(std::make_shared<TitleSpellSuggestions>(word, item));
                }

                LOG_DEBUG << misspelledWords.size() << "title requests";
            }
        }

        if (Common::HasFlag(flags, SuggestionFlags::Description)) {
            for (auto *item: items) {
                QStringList misspelledWords = item->retrieveMisspelledDescriptionWords();
                requests.reserve(requests.size() + misspelledWords.size());

                for (auto &word: misspelledWords) {
                    requests.emplace_back(std::make_shared<DescriptionSpellSuggestions>(word, item));
                }
                LOG_DEBUG << misspelledWords.size() << "description requests";
            }
        }

        return requests;
    }

    SpellCheckSuggestionModel::SpellCheckSuggestionModel(SpellCheckService &spellCheckerService,
                                                         Commands::AppMessages &messages):
        QAbstractListModel(),
        m_SpellCheckerService(spellCheckerService),
        m_Messages(messages)
    {
        m_Messages
                .ofType<Artworks::ArtworksSnapshot>()
                .withID(Commands::AppMessages::SpellSuggestions)
                .addListener(std::bind(&SpellCheckSuggestionModel::setArtworks, this,
                                       std::placeholders::_1));

        m_Messages
                .ofType<Artworks::IMetadataOperator*>()
                .withID(Commands::AppMessages::SpellSuggestions)
                .addListener(std::bind(&SpellCheckSuggestionModel::setupItem,
                                       this,
                                       std::placeholders::_1,
                                       Common::SuggestionFlags::All));
    }

    SpellCheckSuggestionModel::~SpellCheckSuggestionModel() {
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
            std::vector<Artworks::BasicKeywordsModel *> itemsToSubmit(m_CheckedItems.size());

            for (auto &item: m_CheckedItems) {
                auto *metadataOperator = item->getMetadataOperator();
                metadataOperator->afterReplaceCallback();
                itemsToSubmit.push_back(metadataOperator->getBasicKeywordsModel());
            }

            m_SpellCheckerService.submitItems(itemsToSubmit);
        }

        updateItems();
    }

    void SpellCheckSuggestionModel::resetAllSuggestions() {
        LOG_DEBUG << "#";
        for (auto &item: m_SuggestionsList) {
            item->setReplacementIndex(-1);
        }
    }

    void SpellCheckSuggestionModel::setupItem(Artworks::IMetadataOperator *item, Common::SuggestionFlags flags) {
        Q_ASSERT(item != NULL);
        LOG_DEBUG << "#";
        this->setupItems({item}, flags);
    }

    void SpellCheckSuggestionModel::setupItems(const std::vector<Artworks::IMetadataOperator *> &items, Common::SuggestionFlags flags) {
        LOG_INFO << "flags =" << (int)flags;

        auto requests = createSuggestionsRequests(flags, items);
        auto combinedRequests = combineSuggestionRequests(requests);
        LOG_INFO << combinedRequests.size() << "combined request(s)";

        auto executedRequests = setupSuggestions(combinedRequests);
        LOG_INFO << executedRequests.size() << "executed request(s)";

        decltype(m_CheckedItems) lockers(items.size());
        for (auto *item: items) {
            lockers.emplace_back(std::make_shared<Artworks::MetadataOperatorLocker>(item));
        }

#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
        for (auto &executedItem: executedRequests) {
            LOG_INFO << executedItem->toDebugString();
        }
#endif

        beginResetModel();
        {
            m_CheckedItems = std::move(lockers);
            m_SuggestionsList.clear();
            m_SuggestionsList = executedRequests;
        }
        endResetModel();

        emit artworksCountChanged();
    }

    void SpellCheckSuggestionModel::setArtworks(const Artworks::ArtworksSnapshot &snapshot) {
        this->setupItems(
                    Helpers::map<std::shared_ptr<Artworks::ArtworkMetadataLocker>, Artworks::IMetadataOperator*>(
                        snapshot.getRawData(),
                        [](const std::shared_ptr<Artworks::ArtworkMetadataLocker> &locker) {
                        return dynamic_cast<Artworks::IMetadataOperator *>(locker->getArtworkMetadata());
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

    void SpellCheckSuggestionModel::updateItems() const {
        LOG_DEBUG << "#";
        std::vector<int> indices;
        indices.reserve(m_CheckedItems.size());

        for (auto &locker: m_CheckedItems) {
            auto *metadataOperator = locker->getMetadataOperator();
            Artworks::ArtworkMetadata *artwork = dynamic_cast<Artworks::ArtworkMetadata*>(metadataOperator);
            if (artwork != nullptr) {
                indices.push_back(artwork->getLastKnownIndex());
            }
        }

        if (!indices.empty()) {
            m_Messages
                    .ofType<Helpers::IndicesRanges>()
                    .withID(Commands::AppMessages::UpdateArtworks)
                    .broadcast(Helpers::IndicesRanges(indices));
        }
    }
}
