/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "locallibraryqueryengine.h"
#include <QThread>
#include <vector>
#include <memory>
#include "../MetadataIO/metadataioservice.h"
#include "suggestionartwork.h"
#include "../Common/defines.h"

#define MAX_LOCAL_RESULTS 200

namespace Suggestion {
    LocalLibraryQueryEngine::LocalLibraryQueryEngine(int engineID,
                                                     MetadataIO::MetadataIOService *metadataIOService):
        m_EngineID(engineID),
        m_MetadataIOService(metadataIOService)
    {
        Q_ASSERT(metadataIOService != nullptr);

        QObject::connect(&m_Query, &LocalLibraryQuery::resultsReady,
                         this, &LocalLibraryQueryEngine::resultsFoundHandler);
    }

    void LocalLibraryQueryEngine::setSuggestions(std::vector<std::shared_ptr<SuggestionArtwork> > &suggestions) {
        m_Suggestions = std::move(suggestions);
        emit resultsAvailable();
    }

    void LocalLibraryQueryEngine::submitQuery(const Microstocks::SearchQuery &query) {
        LOG_DEBUG << query.getSearchTerms();
        m_Query.setSearchQuery(query);

        m_MetadataIOService->searchArtworks(&m_Query);
    }

    void LocalLibraryQueryEngine::resultsFoundHandler() {
        LOG_DEBUG << "#";
        std::vector<std::shared_ptr<SuggestionArtwork> > results;

        auto &cachedArtworks = m_Query.getResults();
        for (auto &artwork: cachedArtworks) {
            results.emplace_back(new SuggestionArtwork(artwork.m_Filepath, artwork.m_Title, artwork.m_Description, artwork.m_Keywords, true));
        }

        setSuggestions(results);
        m_Query.clear();
    }
}
