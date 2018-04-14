/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MICROSTOCKSUGGESTIONENGINE_H
#define MICROSTOCKSUGGESTIONENGINE_H

#include <memory>
#include "../Microstocks/imicrostockapiclient.h"
#include "../Connectivity/requestsservice.h"
#include "../Microstocks/searchquery.h"
#include "suggestionresultsresponse.h"
#include "isuggestionengine.h"

namespace Suggestion {
    template<class ResponseType>
    class MicrostockSuggestionEngine: public ISuggestionEngine
    {
    public:
        MicrostockSuggestionEngine(int id,
                                   const QString &name,
                                   Microstocks::IMicrostockAPIClient *client,
                                   Connectivity::RequestsService *requestsService):
            m_EngineID(id),
            m_Name(name),
            m_Client(client),
            m_RequestsService(requestsService),
            m_IsCancelled(false)
        {
            Q_ASSERT(client != nullptr);
            Q_ASSERT(requestsService != nullptr);
        }

    public:
        virtual int getID() const override { return m_EngineID; }
        virtual QString getName() const override { return m_Name; }
        virtual int getMaxResultsPerPage() const override { return 100; }

    public:
        virtual void submitQuery(const Microstocks::SearchQuery &query) override {
            m_IsCancelled = false;
            std::shared_ptr<ResponseType> response(new ResponseType(this));
            std::shared_ptr<Connectivity::IConnectivityRequest> request = m_Client->search(query, response);
            m_RequestsService->sendRequest(request);
        }

        virtual void cancelQuery() override {
            m_IsCancelled = true;
        }

    public:
        virtual void setSuggestions(std::vector<std::shared_ptr<SuggestionArtwork> > &suggestions) override {
            if (!m_IsCancelled) {
                m_Suggestions = std::move(suggestions);
                onSuggestionsArrived();
            }
        }

        virtual std::vector<std::shared_ptr<SuggestionArtwork> > &getSuggestions() override { return m_Suggestions; }

    protected:
        virtual void onSuggestionsArrived() = 0;

    private:
        int m_EngineID;
        QString m_Name;
        std::vector<std::shared_ptr<SuggestionArtwork> > m_Suggestions;
        Microstocks::IMicrostockAPIClient *m_Client;
        Connectivity::RequestsService *m_RequestsService;
        volatile bool m_IsCancelled;
    };
}

#endif // MICROSTOCKSUGGESTIONENGINE_H
