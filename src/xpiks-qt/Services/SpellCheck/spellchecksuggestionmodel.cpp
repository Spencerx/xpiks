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

#include <cstddef>
#include <functional>
#include <initializer_list>

#include <QByteArray>
#include <QHash>
#include <QQmlEngine>
#include <QString>
#include <QStringList>
#include <QtDebug>
#include <QtGlobal>

#include "Commands/Base/callbackcommand.h"
#include "Commands/Base/compositecommand.h"
#include "Commands/Editing/clearactionmodeltemplate.h"
#include "Common/flags.h"
#include "Common/logging.h"
#include "Services/SpellCheck/ispellcheckable.h"
#include "Services/SpellCheck/ispellsuggestionstarget.h"
#include "Services/SpellCheck/spellcheckservice.h"
#include "Services/SpellCheck/spellsuggestionsitem.h"

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
                Q_ASSERT(vector.size() == 1);
                result.emplace_back(vector.front());
            }
        }

        return result;
    }

    QHash<ISpellCheckable *, KeywordsSuggestionsVector> combineFailedReplacements(SuggestionsVector const &failedReplacements) {
        QHash<ISpellCheckable *, KeywordsSuggestionsVector> candidatesToRemove;
        size_t size = failedReplacements.size();
        candidatesToRemove.reserve((int)size);

        for (size_t i = 0; i < size; ++i) {
            auto &failedReplacement = failedReplacements.at(i);
            std::shared_ptr<KeywordSpellSuggestions> keywordsItem = std::dynamic_pointer_cast<KeywordSpellSuggestions>(failedReplacement);

            if (keywordsItem) {
                ISpellCheckable *item = keywordsItem->getSpellCheckable();

                if (keywordsItem->isPotentialDuplicate()) {
                    if (!candidatesToRemove.contains(item)) {
                        candidatesToRemove.insert(item, KeywordsSuggestionsVector());
                    }

                    candidatesToRemove[item].emplace_back(keywordsItem);
                }
            } else {
                std::shared_ptr<CombinedSpellSuggestions> combinedItem = std::dynamic_pointer_cast<CombinedSpellSuggestions>(failedReplacement);
                if (combinedItem) {
                    auto keywordsItems = combinedItem->getKeywordsDuplicateSuggestions();

                    for (auto &keywordsCombinedItem: keywordsItems) {
                        ISpellCheckable *item = keywordsCombinedItem->getSpellCheckable();

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

    SpellCheckSuggestionModel::SpellCheckSuggestionModel(SpellCheckService &spellCheckerService):
        QAbstractListModel(),
        m_SpellCheckService(spellCheckerService)
    {
    }

    int SpellCheckSuggestionModel::getArtworksCount() const {
         return m_SpellSuggestionsTarget != nullptr ? (int)m_SpellSuggestionsTarget->size() : 0;
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

    std::shared_ptr<Commands::ICommand> SpellCheckSuggestionModel::getActionCommand(bool yesno) {
        if (yesno) {
            return std::make_shared<Commands::CompositeCommand>(
                        std::initializer_list<std::shared_ptr<Commands::ICommand>>{
                            std::make_shared<Commands::CallbackCommand>(
                            std::bind(&SpellCheckSuggestionModel::submitCorrections, this)),
                            std::make_shared<Commands::ClearActionModelCommand>(*this)});
        } else {
            return std::make_shared<Commands::ClearActionModelCommand>(*this);
        }
    }

    void SpellCheckSuggestionModel::resetModel() {
        LOG_DEBUG << "#";
        beginResetModel();
        {
            m_SuggestionsList.clear();
            m_SpellSuggestionsTarget.reset();
        }
        endResetModel();
        emit artworksCountChanged();
    }

    QObject *SpellCheckSuggestionModel::getSuggestionObject(int index) const {
        SpellSuggestionsItem *item = NULL;

        if (0 <= index && index < (int)m_SuggestionsList.size()) {
            item = m_SuggestionsList.at(index).get();
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        }

        return item;
    }

    void SpellCheckSuggestionModel::resetAllSuggestions() {
        LOG_DEBUG << "#";
        for (auto &item: m_SuggestionsList) {
            item->setReplacementIndex(-1);
        }
    }

#if defined(UI_TESTS) || defined(CORE_TESTS)
    void SpellCheckSuggestionModel::selectSomething() {
        LOG_DEBUG << "#";
        for (auto &suggestion: m_SuggestionsList) {
            suggestion->setReplacementIndex(0);
        }
        emit anythingSelectedChanged();
    }
#endif

    void SpellCheckSuggestionModel::setupModel(const std::shared_ptr<ISpellSuggestionsTarget> &target, Common::SpellCheckFlags flags) {
        LOG_DEBUG << "#";
        m_SpellSuggestionsTarget = target;
        setupRequests(target->generateSuggestionItems(flags));
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
            m_SpellSuggestionsTarget->afterReplaceCallback();
        }
    }

    bool SpellCheckSuggestionModel::processFailedReplacements(const SuggestionsVector &failedReplacements) const {
        LOG_INFO << failedReplacements.size() << "failed items";

        auto candidatesToRemove = combineFailedReplacements(failedReplacements);

        auto it = candidatesToRemove.begin();
        auto itEnd = candidatesToRemove.end();

        bool anyReplaced = false;
        for (; it != itEnd; ++it) {
            ISpellCheckable *item = it.key();

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
            QStringList suggestions = m_SpellCheckService.suggestCorrections(item->getWord());
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
        LOG_INFO << executedRequests.size() << "found suggestions(s)";

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
