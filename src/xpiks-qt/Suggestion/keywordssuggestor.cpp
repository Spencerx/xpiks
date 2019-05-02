/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "keywordssuggestor.h"

#include <algorithm>
#include <cstddef>
#include <functional>

#include <QByteArray>
#include <QHash>
#include <QQmlEngine>
#include <QString>
#include <QVector>
#include <QtDebug>
#include <QtGlobal>

#include "Artworks/basickeywordsmodel.h"
#include "Common/logging.h"
#include "Common/statefulentity.h"
#include "Connectivity/analyticsuserevent.h"
#include "Helpers/constants.h"
#include "Microstocks/microstockenums.h"
#include "Microstocks/searchquery.h"
#include "Models/Editing/quickbuffermessage.h"
#include "Models/settingsmodel.h"
#include "Models/switchermodel.h"
#include "Suggestion/isuggestionengine.h"
#include "Suggestion/suggestionartwork.h"

#ifndef CORE_TESTS
#include "Microstocks/imicrostockapiclients.h"
#include "Suggestion/fotoliasuggestionengine.h"
#include "Suggestion/gettysuggestionengine.h"
#include "Suggestion/locallibraryqueryengine.h"
#include "Suggestion/shutterstocksuggestionengine.h"
#endif

#define LINEAR_TIMER_INTERVAL 1000
#define DEFAULT_SEARCH_TYPE_INDEX 0

namespace Suggestion {
    SuggestionBatches splitSuggestions(SuggestionList const &suggestions, size_t batchSize) {
        SuggestionBatches batches;

        const size_t size = suggestions.size();
        const size_t half = (size / batchSize) / 2;
        const size_t halfSize = half * batchSize;

        for (size_t i = 0; i < halfSize; i += batchSize) {
            auto last = std::min(size, i + batchSize);
            batches.emplace_back(suggestions.begin() + i, suggestions.begin() + last);
        }

        // last batch should be just second half of the data
        batches.emplace_back(suggestions.begin() + halfSize, suggestions.begin() + size);
        Q_ASSERT(suggestions.size() == std::accumulate(batches.begin(), batches.end(), 0,
                                                       [](size_t r, const SuggestionList &l) { return r + l.size(); }));

        return batches;
    }

    KeywordsSuggestor::KeywordsSuggestor(Models::SwitcherModel &switcherModel,
                                         Models::SettingsModel &settingsModel,
                                         Common::ISystemEnvironment &environment,
                                         QObject *parent):
        QAbstractListModel(parent),
        m_State("ksuggest", environment),
        m_SwitcherModel(switcherModel),
        m_SettingsModel(settingsModel),
        m_SuggestedKeywordsModel(this),
        m_OtherKeywordsModel(this),
        m_SelectedArtworksCount(0),
        m_SelectedSourceIndex(0),
        m_LocalSearchIndex(-1),
        m_IsInProgress(false)
    {
        setLastErrorString(tr("No results found"));

        m_ProgressiveLoadTimer.setSingleShot(true);
        QObject::connect(&m_ProgressiveLoadTimer, &QTimer::timeout, this, &KeywordsSuggestor::onProgressiveLoadTimer);

        QObject::connect(&m_SwitcherModel, &Models::SwitcherModel::switchesUpdated,
                         this, &KeywordsSuggestor::onSwitchesUpdated);
    }

    void KeywordsSuggestor::setExistingKeywords(const QSet<QString> &keywords) {
        LOG_DEBUG << "#";
        m_SuggestedKeywords.setExistingKeywords(keywords);
    }

    void KeywordsSuggestor::initSuggestionEngines(Microstocks::IMicrostockAPIClients &microstockClients,
                                                  Connectivity::RequestsService &requestsService,
                                                  MetadataIO::MetadataIOService &metadataIOService) {
        LOG_DEBUG << "#";
#ifndef CORE_TESTS
        int id = 0;
        auto shutterstockEngine = std::make_shared<ShutterstockSuggestionEngine>(
                                      id++,
                                      microstockClients.getClient(Microstocks::MicrostockType::Shutterstock),
                                      requestsService);
        QObject::connect(shutterstockEngine.get(), &ShutterstockSuggestionEngine::resultsAvailable,
                         this, &KeywordsSuggestor::resultsAvailableHandler);
        m_QueryEngines.emplace_back(shutterstockEngine);

        auto gettyEngine = std::make_shared<GettySuggestionEngine>(
                               id++,
                               microstockClients.getClient(Microstocks::MicrostockType::Getty),
                               requestsService);
        QObject::connect(gettyEngine.get(), &GettySuggestionEngine::resultsAvailable,
                         this, &KeywordsSuggestor::resultsAvailableHandler);
        m_QueryEngines.emplace_back(gettyEngine);

        // https://github.com/ribtoks/xpiks/issues/463
        gettyEngine->setIsEnabled(m_SwitcherModel.getGettySuggestionEnabled());

        auto fotoliaEngine = std::make_shared<FotoliaSuggestionEngine>(
                                 id++,
                                 microstockClients.getClient(Microstocks::MicrostockType::Fotolia),
                                 requestsService);
        QObject::connect(fotoliaEngine.get(), &FotoliaSuggestionEngine::resultsAvailable,
                         this, &KeywordsSuggestor::resultsAvailableHandler);
        m_QueryEngines.emplace_back(fotoliaEngine);

        auto localEngine = std::make_shared<LocalLibraryQueryEngine>(
                               id++,
                               metadataIOService);
        QObject::connect(localEngine.get(), &LocalLibraryQueryEngine::resultsAvailable,
                         this, &KeywordsSuggestor::resultsAvailableHandler);
        m_QueryEngines.emplace_back(localEngine);
        m_LocalSearchIndex = static_cast<int>(m_QueryEngines.size()) - 1;
#endif
        m_State.init();
    }

    void KeywordsSuggestor::setSuggestedArtworks(std::vector<std::shared_ptr<SuggestionArtwork>> &suggestedArtworks) {
        LOG_INFO << suggestedArtworks.size() << "item(s)";

        setSelectedArtworksCount(0);
        m_SuggestedKeywords.reset();
        m_SuggestedKeywordsModel.clearKeywords();
        m_OtherKeywordsModel.clearKeywords();
        m_PendingSuggestions.clear();

#if !defined(INTEGRATION_TESTS) && !defined(CORE_TESTS)
        const bool sequentialLoading =
                (m_SwitcherModel.getProgressiveSuggestionPreviewsOn() ||
                 m_SettingsModel.getUseProgressiveSuggestionPreviews()) &&
                (!getIsLocalSearch());
#else
        const bool sequentialLoading = false;
#endif
        LOG_INFO << "With sequential loading:" << sequentialLoading;

        if (sequentialLoading) {
            const int increment = m_SettingsModel.getProgressiveSuggestionIncrement();
            m_PendingSuggestions = splitSuggestions(suggestedArtworks, static_cast<size_t>(increment));
            LOG_DEBUG << "Progressive increment is" << increment;
        } else {
            m_PendingSuggestions.emplace_back(std::move(suggestedArtworks));
        }

        beginResetModel();
        {
            m_Suggestions.clear();
            m_Suggestions = m_PendingSuggestions.at(0);
            m_PendingSuggestions.pop_front();
        }
        endResetModel();

        if (m_PendingSuggestions.size() > 0) {
            m_ProgressiveLoadTimer.start(LINEAR_TIMER_INTERVAL);
        }

        unsetInProgress();
        emit suggestionArrived();
        emit suggestedKeywordsCountChanged();
        emit otherKeywordsCountChanged();
        emit selectedArtworksCountChanged();
    }

    void KeywordsSuggestor::clear() {
        LOG_DEBUG << "#";

        setSelectedArtworksCount(0);
        m_SuggestedKeywords.reset();
        m_SuggestedKeywordsModel.clearKeywords();
        m_OtherKeywordsModel.clearKeywords();
        m_PendingSuggestions.clear();

        beginResetModel();
        {
            m_Suggestions.clear();
        }
        endResetModel();

        unsetInProgress();
        emit suggestedKeywordsCountChanged();
        emit otherKeywordsCountChanged();
        emit selectedArtworksCountChanged();
    }

    void KeywordsSuggestor::setSelectedSourceIndex(int value) {
        if (!m_IsInProgress && (value != m_SelectedSourceIndex)) {
            if (0 <= value &&
                    value < (int)m_QueryEngines.size()) {
                LOG_INFO << "Selected query source index:" << value;
                m_SelectedSourceIndex = value;
                emit selectedSourceIndexChanged();
                emit isLocalSearchChanged();
            }
        }
    }

    int KeywordsSuggestor::getSearchTypeIndex() const {
        return m_State.getInt(Constants::suggestorSearchTypeIndex, DEFAULT_SEARCH_TYPE_INDEX);
    }

    void KeywordsSuggestor::setSearchTypeIndex(int value) {
        int current = getSearchTypeIndex();
        if (current != value) {
            m_State.setValue(Constants::suggestorSearchTypeIndex, value);
            emit searchTypeIndexChanged();
            m_State.sync();
        }
    }

    void KeywordsSuggestor::setSelectedArtworksCount(int value) {
        if (m_SelectedArtworksCount != value) {
            m_SelectedArtworksCount = value;
            emit selectedArtworksCountChanged();
        }
    }

    bool KeywordsSuggestor::getIsLocalSearch() const {
        bool result = m_SelectedSourceIndex == m_LocalSearchIndex;
        Q_ASSERT(result == isLocalSuggestionActive());
        return result;
    }

    void KeywordsSuggestor::resultsAvailableHandler() {
        LOG_INFO << "#";
        auto engine = getSelectedEngine();
        unsetInProgress();
        if (engine) {
            auto &results = engine->getSuggestions();
            setSuggestedArtworks(results);
        }
    }

    void KeywordsSuggestor::errorsReceivedHandler(const QString &error) {
        unsetInProgress();
        setLastErrorString(error);
    }

    void KeywordsSuggestor::onProgressiveLoadTimer() {
        LOG_DEBUG << "#";
        if (m_PendingSuggestions.empty()) {
            LOG_DEBUG << "Pending suggestions are empty";
            return;
        }

        auto batch = m_PendingSuggestions.at(0);
        LOG_DEBUG << "Loading from" << m_Suggestions.size() << "to" << (m_Suggestions.size() + batch.size());

        beginInsertRows(QModelIndex(),
                        static_cast<int>(m_Suggestions.size()),
                        static_cast<int>(m_Suggestions.size() + batch.size() - 1));
        {
            m_Suggestions.insert(m_Suggestions.end(), batch.begin(), batch.end());
        }
        endInsertRows();

        m_PendingSuggestions.pop_front();
        m_ProgressiveLoadTimer.start(LINEAR_TIMER_INTERVAL);
    }

    void KeywordsSuggestor::onSwitchesUpdated() {
        LOG_DEBUG << "#";
        bool found = false;
#ifndef CORE_TESTS
        for (auto &engine: m_QueryEngines) {
            auto gettyEngine = std::dynamic_pointer_cast<GettySuggestionEngine>(engine);
            if (gettyEngine) {
                gettyEngine->setIsEnabled(m_SwitcherModel.getGettySuggestionEnabled());
                found = true;
                break;
            }
        }
#endif

        if (!found) {
            LOG_WARNING << "Failed to find Getty engine";
        } else {
            emit engineNamesChanged();
        }
    }

    void KeywordsSuggestor::onLanguageChanged() {
        setLastErrorString(tr("No results found"));
        emit engineNamesChanged();
    }

    bool KeywordsSuggestor::isLocalSuggestionActive() const {
        const int index = getSelectedSourceIndex();
        if ((index < 0) || (index >= (int)m_QueryEngines.size())) { return false; }

        bool result = false;
#ifndef CORE_TESTS
        auto &engine = m_QueryEngines.at(index);
        auto localEngine = std::dynamic_pointer_cast<LocalLibraryQueryEngine>(engine);
        result = localEngine != nullptr;
#endif
        return result;
    }

    void KeywordsSuggestor::appendKeywordToSuggested(const QString &keyword) {
        m_SuggestedKeywordsModel.appendKeyword(keyword);
        emit suggestedKeywordsCountChanged();
    }

    void KeywordsSuggestor::appendKeywordToOther(const QString &keyword) {
        m_OtherKeywordsModel.appendKeyword(keyword);
        emit otherKeywordsCountChanged();
    }

    QString KeywordsSuggestor::removeSuggestedKeywordAt(int keywordIndex) {
        LOG_INFO << "index:" << keywordIndex;

        QString keyword;
        if (m_SuggestedKeywordsModel.removeKeywordAt(keywordIndex, keyword)) {
            emit suggestedKeywordsCountChanged();
            LOG_INFO << "Removed:" << keyword;
        }

        return keyword;
    }

    QString KeywordsSuggestor::removeOtherKeywordAt(int keywordIndex) {
        LOG_INFO << "index:" << keywordIndex;

        QString keyword;
        if (m_OtherKeywordsModel.removeKeywordAt(keywordIndex, keyword)) {
            emit otherKeywordsCountChanged();
            LOG_INFO << "Removed:" << keyword;
        }
        return keyword;
    }

    void KeywordsSuggestor::setArtworkSelected(int index, bool newState) {
        LOG_INFO << index << newState;
        if (index < 0 || (size_t)index >= m_Suggestions.size()) {
            return;
        }

        auto &suggestionArtwork = m_Suggestions.at(index);
        suggestionArtwork->setIsSelected(newState);

        if (newState) {
            m_SuggestedKeywords.addKeywords(suggestionArtwork->getKeywordsSet());
        } else {
            m_SuggestedKeywords.removeKeywords(suggestionArtwork->getKeywordsSet());
        }

        int sign = newState ? +1 : -1;
        setSelectedArtworksCount(m_SelectedArtworksCount + sign);

        QModelIndex qIndex = this->index(index);
        emit dataChanged(qIndex, qIndex, QVector<int>() << IsSelectedRole);
        updateSuggestedKeywords();
    }

    void KeywordsSuggestor::copyToQuickBuffer(int index) const {
        if (index < 0 || (size_t)index >= m_Suggestions.size()) {
            LOG_WARNING << "Index is out of bounds: " << index;
            return;
        }

        auto &suggestionArtwork = m_Suggestions.at(index);

        sendMessage(
                    Models::QuickBufferMessage(
                        QString(suggestionArtwork->getTitle()),
                        QString(suggestionArtwork->getDescription()),
                        suggestionArtwork->getKeywordsSet().toList(),
                        true));
    }

    void KeywordsSuggestor::searchArtworks(const QString &searchTerm, int resultsType) {
        LOG_INFO << "[" << searchTerm << "], search type:" << resultsType;

        if (!m_IsInProgress && !searchTerm.trimmed().isEmpty()) {
            setInProgress();

            auto engine = getSelectedEngine();
            if (engine) {
                Microstocks::SearchQuery query(searchTerm, resultsType, engine->getMaxResultsPerPage());
                engine->submitQuery(query);
                m_ProgressiveLoadTimer.stop();
#ifndef CORE_TESTS
                if (std::dynamic_pointer_cast<LocalLibraryQueryEngine>(engine) == nullptr) {
                    sendMessage(Connectivity::EventType::SuggestionRemote);
                } else {
                    sendMessage(Connectivity::EventType::SuggestionLocal);
                }
#endif
            } else {
                LOG_WARNING << "Selected engine is not found";
            }
        }
    }

    void KeywordsSuggestor::cancelSearch() {
        LOG_DEBUG << "#";
        auto engine = getSelectedEngine();
        if (engine) {
            engine->cancelQuery();
        }
        m_ProgressiveLoadTimer.stop();
        sendMessage(Connectivity::EventType::SuggestionCancel);
    }

    QStringList KeywordsSuggestor::getEngineNames() const {
        QStringList names;
        for (auto &engine: m_QueryEngines) {
            if (!engine->getIsEnabled()) { continue; }
            names.append(engine->getName());
        }
        return names;
    }

    void KeywordsSuggestor::clearSuggested() {
        LOG_DEBUG << "#";

        while (!m_SuggestedKeywordsModel.isEmpty()) {
            QString keyword = removeSuggestedKeywordAt(0);
            appendKeywordToOther(keyword);
        }
    }

    void KeywordsSuggestor::resetSelection() {
        LOG_DEBUG << "#";
        setSelectedArtworksCount(0);
        m_SuggestedKeywords.reset();
        m_SuggestedKeywordsModel.clearKeywords();
        m_OtherKeywordsModel.clearKeywords();

        beginResetModel();
        {
            for (auto &item: m_Suggestions) {
                item->setIsSelected(false);
            }
        }
        endResetModel();

        unsetInProgress();
        emit suggestedKeywordsCountChanged();
        emit otherKeywordsCountChanged();
        emit selectedArtworksCountChanged();
    }

    QObject *KeywordsSuggestor::getSuggestedKeywordsModel() {
        QObject *item = &m_SuggestedKeywordsModel;
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        return item;
    }

    QObject *KeywordsSuggestor::getAllOtherKeywordsModel() {
        QObject *item = &m_OtherKeywordsModel;
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        return item;
    }

    int KeywordsSuggestor::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return static_cast<int>(m_Suggestions.size());
    }

    QVariant KeywordsSuggestor::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if (row < 0 || (size_t)row >= m_Suggestions.size()) { return QVariant(); }

        auto &suggestionArtwork = m_Suggestions.at(row);

        switch (role) {
        case UrlRole:
            return suggestionArtwork->getUrl();
        case IsSelectedRole:
            return suggestionArtwork->getIsSelected();
        case ExternalUrlRole:
            return suggestionArtwork->getExternalUrl();
        default:
            return QVariant();
        }
    }

    QHash<int, QByteArray> KeywordsSuggestor::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[UrlRole] = "url";
        roles[IsSelectedRole] = "isselected";
        roles[ExternalUrlRole] = "externalurl";
        return roles;
    }

    QSet<QString> KeywordsSuggestor::getSelectedArtworksKeywords() const {
        QSet<QString> allKeywords;
        size_t size = m_Suggestions.size();

        for (size_t i = 0; i < size; ++i) {
            auto &artwork = m_Suggestions.at(i);

            if (artwork->getIsSelected()) {
                const QSet<QString> &currentKeywords = artwork->getKeywordsSet();
                allKeywords.unite(currentKeywords);
            }
        }

        return allKeywords;
    }

    void KeywordsSuggestor::updateSuggestedKeywords() {
        m_SuggestedKeywords.updateSuggestion(m_SelectedArtworksCount);

        m_SuggestedKeywordsModel.setKeywords(m_SuggestedKeywords.getSuggestedKeywords());
        m_OtherKeywordsModel.setKeywords(m_SuggestedKeywords.getOtherKeywords());

        emit suggestedKeywordsCountChanged();
        emit otherKeywordsCountChanged();
    }

    std::shared_ptr<ISuggestionEngine> KeywordsSuggestor::getSelectedEngine() {
        const int selectedIndex = m_SelectedSourceIndex;
        int index = 0;
        for (auto &engine: m_QueryEngines) {
            if (engine->getIsEnabled() == false) { continue; }
            if (index == selectedIndex) { break; }
            index++;
        }

        std::shared_ptr<ISuggestionEngine> result;

        if ((0 <= index) && (index < (int)m_QueryEngines.size())) {
            result = m_QueryEngines[index];
        }  else {
            LOG_WARNING << "Failed to find current engine by index" << selectedIndex;
        }

        return result;
    }
}
