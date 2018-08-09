/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SHUTTERSTOCKSUGGESTIONRESULTS_H
#define SHUTTERSTOCKSUGGESTIONRESULTS_H

#include <QObject>
#include "suggestionresultsresponse.h"
#include "microstocksuggestionengine.h"

namespace Suggestion {
    class ShutterstockSuggestionResults: public SuggestionResultsResponse
    {
    public:
        ShutterstockSuggestionResults(ISuggestionsRepository &suggestions):
            SuggestionResultsResponse(suggestions)
        { }

        // SuggestionResultsResponse interface
    protected:
        virtual bool parseResponse(const QByteArray &body, std::vector<std::shared_ptr<SuggestionArtwork> > &artworks) override;
    };

    class ShutterstockSuggestionEngine:
            public QObject,
            public MicrostockSuggestionEngine<ShutterstockSuggestionResults>
    {
        Q_OBJECT
    public:
        ShutterstockSuggestionEngine(int id,
                                     Microstocks::IMicrostockAPIClient &client,
                                     Connectivity::RequestsService &requestsService):
            MicrostockSuggestionEngine(id,
                                       "Shutterstock",
                                       client,
                                       requestsService)
        { }

    protected:
        virtual void onSuggestionsArrived() override { emit resultsAvailable(); }

    signals:
        void resultsAvailable();
    };
}

#endif // SHUTTERSTOCKSUGGESTIONRESULTS_H
