/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef FOTOLIASUGGESTIONRESULTS_H
#define FOTOLIASUGGESTIONRESULTS_H

#include <memory>
#include <vector>

#include <QByteArray>
#include <QObject>
#include <QString>

#include "Suggestion/microstocksuggestionengine.h"
#include "Suggestion/suggestionresultsresponse.h"

namespace Connectivity {
    class RequestsService;
}

namespace Microstocks {
    class IMicrostockAPIClient;
}

namespace Suggestion {
    class ISuggestionsRepository;
    class SuggestionArtwork;

    class FotoliaSuggestionResults: public SuggestionResultsResponse
    {
    public:
        FotoliaSuggestionResults(ISuggestionsRepository &suggestions):
            SuggestionResultsResponse(suggestions)
        { }

        // SuggestionResultsResponse interface
    protected:
        virtual bool parseResponse(const QByteArray &body, std::vector<std::shared_ptr<SuggestionArtwork> > &artworks) override;
    };

    class FotoliaSuggestionEngine: public QObject, public MicrostockSuggestionEngine<FotoliaSuggestionResults> {
        Q_OBJECT
    public:
        FotoliaSuggestionEngine(int id,
                                     std::shared_ptr<Microstocks::IMicrostockAPIClient> const &client,
                                     Connectivity::RequestsService &requestsService):
            MicrostockSuggestionEngine(id,
                                       "Fotolia",
                                       client,
                                       requestsService)
        { }

    protected:
        virtual void onSuggestionsArrived() override { emit resultsAvailable(); }

    signals:
        void resultsAvailable();
    };
}

#endif // FOTOLIASUGGESTIONRESULTS_H
