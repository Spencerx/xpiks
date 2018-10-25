/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SUGGESTIONRESULTSRESPONSE_H
#define SUGGESTIONRESULTSRESPONSE_H

#include <memory>
#include <vector>

#include <QByteArray>

#include "Connectivity/iconnectivityresponse.h"

namespace Suggestion {
    class ISuggestionsRepository;
    class SuggestionArtwork;

    class SuggestionResultsResponse: public Connectivity::IConnectivityResponse {
    public:
        SuggestionResultsResponse(ISuggestionsRepository &suggestions);

        // IConnectivityResponse interface
    public:
        virtual void setResult(bool result, const QByteArray &body) override;

    protected:
        virtual bool parseResponse(const QByteArray &body, std::vector<std::shared_ptr<SuggestionArtwork> > &artworks) = 0;
        virtual void onResultsAvailable();

    public:
        std::vector<std::shared_ptr<SuggestionArtwork> > &getSuggestedArtworks() { return m_SuggestedArtworks; }

    private:
        std::vector<std::shared_ptr<SuggestionArtwork> > m_SuggestedArtworks;
        ISuggestionsRepository &m_Suggestions;
    };
}

#endif // SUGGESTIONRESULTSRESPONSE_H
