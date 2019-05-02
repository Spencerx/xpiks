/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GETTYSUGGESTIONRESULTS_H
#define GETTYSUGGESTIONRESULTS_H

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

    class GettySuggestionResults : public SuggestionResultsResponse
    {
    public:
        GettySuggestionResults(ISuggestionsRepository &suggestions):
            SuggestionResultsResponse(suggestions)
        { }

        // SuggestionResultsResponse interface
    protected:
        virtual bool parseResponse(const QByteArray &body, std::vector<std::shared_ptr<SuggestionArtwork> > &artworks) override;
    };

    class GettySuggestionEngine:
            public QObject,
            public MicrostockSuggestionEngine<GettySuggestionResults>
    {
        Q_OBJECT
    public:
        GettySuggestionEngine(int id,
                              std::shared_ptr<Microstocks::IMicrostockAPIClient> const &client,
                              Connectivity::RequestsService &requestsService):
            MicrostockSuggestionEngine(id,
                                       "iStock",
                                       client,
                                       requestsService)
        { }

    protected:
        virtual void onSuggestionsArrived(bool success) override { emit resultsAvailable(success); }

    signals:
        void resultsAvailable(bool success);
    };
}

#endif // GETTYSUGGESTIONRESULTS_H
