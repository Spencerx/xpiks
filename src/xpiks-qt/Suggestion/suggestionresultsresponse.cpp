/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "suggestionresultsresponse.h"

#include "Suggestion/isuggestionsrepository.h"

namespace Suggestion {
    SuggestionResultsResponse::SuggestionResultsResponse(ISuggestionsRepository &suggestions):
        m_Suggestions(suggestions)
    {
    }

    void SuggestionResultsResponse::setResult(bool result, const QByteArray &body) {
        if (result) {
            decltype(m_SuggestedArtworks) artworks;
            if (parseResponse(body, artworks)) {
                m_SuggestedArtworks.swap(artworks);
            }
        }

        onResultsAvailable();
    }

    void SuggestionResultsResponse::onResultsAvailable() {
        m_Suggestions.setSuggestions(m_SuggestedArtworks);
    }
}
