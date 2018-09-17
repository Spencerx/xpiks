/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QHash>
#include <QString>
#include <QMultiMap>
#include <QQmlEngine>
#include "keywordssuggestor.h"
#include "suggestionartwork.h"
#include <Common/defines.h>
#include <Models/switchermodel.h>
#include <Models/settingsmodel.h>
#include <Helpers/constants.h>
#include <Connectivity/analyticsuserevent.h>

#ifndef CORE_TESTS
#include "locallibraryqueryengine.h"
#include "shutterstocksuggestionengine.h"
#include "fotoliasuggestionengine.h"
#include "gettysuggestionengine.h"
#include <Microstocks/imicrostockapiclients.h>
#endif

#define LINEAR_TIMER_INTERVAL 1000
#define DEFAULT_SEARCH_TYPE_INDEX 0

namespace Suggestion {
    KeywordsSuggestor::KeywordsSuggestor(Models::SwitcherModel &switcherModel,
                                         Models::SettingsModel &settingsModel,
                                         Common::ISystemEnvironment &environment,
                                         QObject *parent):
        QAbstractListModel(parent),
        m_State("ksuggest", environment),
        m_SwitcherModel(switcherModel),
        m_SettingsModel(settingsModel),
        m_SuggestedKeywords(this),
        m_AllOtherKeywords(this),
        m_SelectedArtworksCount(0),
        m_SelectedSourceIndex(0),
        m_LocalSearchIndex(-1),
        m_IsInProgress(false)
    {
        setLastErrorString(tr("No results found"));
        qsrand(QTime::currentTime().msec());

        m_LinearTimer.setSingleShot(true);
        QObject::connect(&m_LinearTimer, &QTimer::timeout, this, &KeywordsSuggestor::onLinearTimer);

        QObject::connect(&m_SwitcherModel, &Models::SwitcherModel::switchesUpdated,
                         this, &KeywordsSuggestor::onSwitchesUpdated);
    }

    void KeywordsSuggestor::setExistingKeywords(const QSet<QString> &keywords) {
        LOG_DEBUG << "#";
        m_ExistingKeywords.clear(); m_ExistingKeywords.unite(keywords);
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
        m_LocalSearchIndex = (int)m_QueryEngines.size() - 1;
#endif
        m_State.init();
    }

    void KeywordsSuggestor::setSuggestedArtworks(std::vector<std::shared_ptr<SuggestionArtwork> > &suggestedArtworks) {
        LOG_INFO << suggestedArtworks.size() << "item(s)";

        m_SelectedArtworksCount = 0;
        m_KeywordsHash.clear();
        m_SuggestedKeywords.clearKeywords();
        m_AllOtherKeywords.clearKeywords();

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
            m_LoadedPreviewsNumber = increment;
            LOG_DEBUG << "Progressive increment is" << increment;
        } else {
            m_LoadedPreviewsNumber = (int)suggestedArtworks.size();
        }

        beginResetModel();
        {
            m_Suggestions = std::move(suggestedArtworks);
        }
        endResetModel();

        if (sequentialLoading) {
            m_LinearTimer.start(LINEAR_TIMER_INTERVAL);
        }

        unsetInProgress();
        emit suggestionArrived();
        emit suggestedKeywordsCountChanged();
        emit otherKeywordsCountChanged();
        emit selectedArtworksCountChanged();
    }

    void KeywordsSuggestor::clear() {
        LOG_DEBUG << "#";

        m_SelectedArtworksCount = 0;
        m_KeywordsHash.clear();
        m_SuggestedKeywords.clearKeywords();
        m_AllOtherKeywords.clearKeywords();
        m_ExistingKeywords.clear();
        m_LoadedPreviewsNumber = 0;

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

    bool KeywordsSuggestor::getIsLocalSearch() const {
        bool result = m_SelectedSourceIndex == m_LocalSearchIndex;
        Q_ASSERT(result == isLocalSuggestionActive());
        return result;
    }

    void KeywordsSuggestor::resultsAvailableHandler() {
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

    void KeywordsSuggestor::onLinearTimer() {
        const int size = rowCount();
        if (m_LoadedPreviewsNumber >= size) { return; }

        const int increment = m_SettingsModel.getProgressiveSuggestionIncrement();

        QModelIndex firstIndex = this->index(m_LoadedPreviewsNumber);
        int nextIndex = m_LoadedPreviewsNumber + increment;
        if (nextIndex > size/2) {
            nextIndex = size - 1;
        }

        QModelIndex lastIndex = this->index(nextIndex);
        m_LoadedPreviewsNumber = nextIndex;

        emit dataChanged(firstIndex, lastIndex, QVector<int>() << UrlRole);
        m_LinearTimer.start(LINEAR_TIMER_INTERVAL);
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

    QString KeywordsSuggestor::removeSuggestedKeywordAt(int keywordIndex) {
        LOG_INFO << "Index:" << keywordIndex;

        QString keyword;
        if (m_SuggestedKeywords.removeKeywordAt(keywordIndex, keyword)) {
            emit suggestedKeywordsCountChanged();
            LOG_INFO << "Removed:" << keyword;
        }

        return keyword;
    }

    QString KeywordsSuggestor::removeOtherKeywordAt(int keywordIndex) {
        LOG_INFO << "Index:" << keywordIndex;

        QString keyword;
        if (m_AllOtherKeywords.removeKeywordAt(keywordIndex, keyword)) {
            emit otherKeywordsCountChanged();
            LOG_INFO << "Removed:" << keyword;
        }
        return keyword;
    }

    void KeywordsSuggestor::setArtworkSelected(int index, bool newState) {
        if (index < 0 || (size_t)index >= m_Suggestions.size()) {
            return;
        }

        auto &suggestionArtwork = m_Suggestions.at(index);
        suggestionArtwork->setIsSelected(newState);

        int sign = newState ? +1 : -1;
        accountKeywords(suggestionArtwork->getKeywordsSet(), sign);
        m_SelectedArtworksCount += sign;
        emit selectedArtworksCountChanged();

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
#ifndef CORE_TESTS
                if (std::dynamic_pointer_cast<LocalLibraryQueryEngine>(engine) == nullptr) {
                    sendMessage(Connectivity::UserAction::SuggestionRemote);
                } else {
                    sendMessage(Connectivity::UserAction::SuggestionLocal);
                }
#endif
            }
        }
    }

    void KeywordsSuggestor::cancelSearch() {
        LOG_DEBUG << "#";
        auto engine = getSelectedEngine();
        if (engine) {
            engine->cancelQuery();
        }
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

        while (!m_SuggestedKeywords.isEmpty()) {
            QString keyword = removeSuggestedKeywordAt(0);
            appendKeywordToOther(keyword);
        }
    }

    void KeywordsSuggestor::resetSelection() {
        LOG_DEBUG << "#";
        m_SelectedArtworksCount = 0;
        m_KeywordsHash.clear();
        m_SuggestedKeywords.clearKeywords();
        m_AllOtherKeywords.clearKeywords();
        m_ExistingKeywords.clear();

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
        QObject *item = &m_SuggestedKeywords;
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        return item;
    }

    QObject *KeywordsSuggestor::getAllOtherKeywordsModel() {
        QObject *item = &m_AllOtherKeywords;
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        return item;
    }

    int KeywordsSuggestor::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return (int)m_Suggestions.size();
    }

    QVariant KeywordsSuggestor::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if (row < 0 || (size_t)row >= m_Suggestions.size()) { return QVariant(); }

        auto &suggestionArtwork = m_Suggestions.at(row);

        switch (role) {
        case UrlRole: {
#ifdef QT_DEBUG
            if (row <= m_LoadedPreviewsNumber) {
                return suggestionArtwork->getUrl();
            } else {
                return QString("");
            }
#else
            return suggestionArtwork->getUrl();
#endif
        }
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

    void KeywordsSuggestor::accountKeywords(const QSet<QString> &keywords, int sign) {
        foreach(const QString &keyword, keywords) {
            if (m_KeywordsHash.contains(keyword)) {
                m_KeywordsHash[keyword] += sign;
            } else {
                m_KeywordsHash.insert(keyword, 1);
            }
        }
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
        QStringList suggestedKeywords, otherKeywords;
        QMultiMap<int, QString> selectedKeywords;
        int lowerThreshold, upperThreshold;
        calculateBounds(lowerThreshold, upperThreshold);

        QHash<QString, int>::const_iterator hashIt = m_KeywordsHash.constBegin();
        QHash<QString, int>::const_iterator hashItEnd = m_KeywordsHash.constEnd();

        for (; hashIt != hashItEnd; ++hashIt) {
            selectedKeywords.insert(hashIt.value(), hashIt.key());
        }

        QMultiMap<int, QString>::const_iterator it = selectedKeywords.constEnd();
        QMultiMap<int, QString>::const_iterator itBegin = selectedKeywords.constBegin();

        int maxSuggested = 35 + (qrand() % 10);
        int maxUpperBound = 40 + (qrand() % 5);
        int maxOthers = 35 + (qrand() % 10);

        suggestedKeywords.reserve(maxSuggested);
        otherKeywords.reserve(maxOthers);

        bool canAddToSuggested, canAddToOthers;
        const bool isOnlyOneArtwork = (m_SelectedArtworksCount == 1);

        while (it != itBegin) {
            --it;

            int frequency = it.key();
            const QString &frequentKeyword = it.value();

            if (frequency == 0) { continue; }
            if (m_ExistingKeywords.contains(frequentKeyword.toLower())) {
                LOG_DEBUG << "Skipping existing keyword" << frequentKeyword;
                continue;
            }

            int suggestedCount = suggestedKeywords.length();

            canAddToSuggested = (frequency >= upperThreshold) && (suggestedCount <= maxUpperBound);
            canAddToOthers = frequency >= lowerThreshold;

            if (isOnlyOneArtwork || canAddToSuggested ||
                    (canAddToOthers && (suggestedCount <= maxSuggested))) {
                suggestedKeywords.append(frequentKeyword);
            } else if (canAddToOthers || (otherKeywords.length() <= maxOthers)) {
                otherKeywords.append(frequentKeyword);

                if (otherKeywords.length() > maxOthers) {
                    break;
                }
            }
        }

        m_SuggestedKeywords.setKeywords(suggestedKeywords);
        m_AllOtherKeywords.setKeywords(otherKeywords);

        emit suggestedKeywordsCountChanged();
        emit otherKeywordsCountChanged();
    }

    void KeywordsSuggestor::calculateBounds(int &lowerBound, int &upperBound) const {
        if (m_SelectedArtworksCount <= 2) {
            lowerBound = 1;
            upperBound = qMax(m_SelectedArtworksCount, 1);
        } else if (m_SelectedArtworksCount <= 4) {
            lowerBound = 2;
            upperBound = 3;
        } else if (m_SelectedArtworksCount <= 5) {
            lowerBound = 2;
            upperBound = 3;
        } else if (m_SelectedArtworksCount <= 9) {
            upperBound = m_SelectedArtworksCount / 2;
            lowerBound = upperBound - 1;
        } else if (m_SelectedArtworksCount <= 15) {
            upperBound = m_SelectedArtworksCount / 2 - 1;
            lowerBound = upperBound - 2;
        } else {
            upperBound = m_SelectedArtworksCount / 2;
            lowerBound = upperBound - 2;
        }
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
