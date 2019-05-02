/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MICROSTOCKSUGGESTIONENGINE_H
#define MICROSTOCKSUGGESTIONENGINE_H

#include <atomic>
#include <memory>
#include <vector>
#include <utility>

#include <QString>
#include <QtGlobal>

#include "Connectivity/requestsservice.h"
#include "Microstocks/imicrostockapiclient.h"
#include "Suggestion/isuggestionengine.h"

namespace Connectivity {
    class IConnectivityRequest;
}

namespace Microstocks {
    class SearchQuery;
}

namespace Suggestion {
    class SuggestionArtwork;

    template<class ResponseType>
    class MicrostockSuggestionEngine: public ISuggestionEngine
    {
    public:
        MicrostockSuggestionEngine(int id,
                                   const QString &name,
                                   std::shared_ptr<Microstocks::IMicrostockAPIClient> const &client,
                                   Connectivity::RequestsService &requestsService):
            m_EngineID(id),
            m_Name(name),
            m_Client(client),
            m_RequestsService(requestsService),
            m_IsCancelled(false),
            m_IsEnabled(true)
        {
#ifndef CORE_TESTS
            Q_ASSERT(client != nullptr);
#endif
        }

    public:
        virtual int getID() const override { return m_EngineID; }
        virtual QString getName() const override { return m_Name; }
        virtual int getMaxResultsPerPage() const override { return 100; }
        virtual bool getIsEnabled() const override { return m_IsEnabled; }
        virtual void setIsEnabled(bool value) override { m_IsEnabled = value; }

    public:
        virtual void submitQuery(const Microstocks::SearchQuery &query) override {
            Q_ASSERT(m_IsEnabled);
            if (!m_IsEnabled) { return; }
            m_IsCancelled = false;
            auto response = std::make_shared<ResponseType>(*this);
            std::shared_ptr<Connectivity::IConnectivityRequest> request = m_Client->search(query, response);
            m_RequestsService.sendRequest(request);
        }

        virtual void cancelQuery() override {
            m_IsCancelled = true;
        }

    public:
        virtual void setSuggestions(std::vector<std::shared_ptr<SuggestionArtwork> > &suggestions) override {
            if (!m_IsCancelled) {
                m_Suggestions = std::move(suggestions);
                onSuggestionsArrived(true);
            }
        }
        virtual std::vector<std::shared_ptr<SuggestionArtwork> > &getSuggestions() override { return m_Suggestions; }
        virtual void setError() override { onSuggestionsArrived(false); }

    protected:
        virtual void onSuggestionsArrived(bool success) = 0;

    private:
        int m_EngineID;
        QString m_Name;
        std::vector<std::shared_ptr<SuggestionArtwork> > m_Suggestions;
        std::shared_ptr<Microstocks::IMicrostockAPIClient> m_Client;
        Connectivity::RequestsService &m_RequestsService;
        std::atomic_bool m_IsCancelled;
        std::atomic_bool m_IsEnabled;
    };
}

#endif // MICROSTOCKSUGGESTIONENGINE_H
