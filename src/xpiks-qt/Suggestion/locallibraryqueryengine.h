/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LOCALLIBRARYQUERYENGINE_H
#define LOCALLIBRARYQUERYENGINE_H

#include <memory>
#include <vector>

#include <QObject>
#include <QString>

#include "Suggestion/locallibraryquery.h"
#include "Suggestion/isuggestionengine.h"

namespace Microstocks {
    class SearchQuery;
}

namespace MetadataIO {
    class MetadataIOService;
}

namespace Suggestion {
    class SuggestionArtwork;

    class LocalLibraryQueryEngine:
            public QObject,
            public ISuggestionEngine
    {
        Q_OBJECT
    public:
        LocalLibraryQueryEngine(int engineID,
                                MetadataIO::MetadataIOService &metadataIOService);

    public:
        virtual void setSuggestions(std::vector<std::shared_ptr<SuggestionArtwork> > &suggestions) override;
        virtual void setError() override;
        virtual std::vector<std::shared_ptr<SuggestionArtwork> > &getSuggestions() override { return m_Suggestions; }

        // ISuggestionEngine interface
    public:
        virtual QString getName() const override { return tr("Local files"); }
        virtual int getID() const override { return m_EngineID; }
        virtual int getMaxResultsPerPage() const override { return 200; }
        virtual bool getIsEnabled() const override { return m_IsEnabled; }
        virtual void setIsEnabled(bool value) override { m_IsEnabled = value; }

    public:
        virtual void submitQuery(const Microstocks::SearchQuery &query) override;
        virtual void cancelQuery() override { /* BUMP */ }

    private slots:
        void resultsFoundHandler();

    signals:
        void resultsAvailable(bool success);

    private:
        int m_EngineID;
        LocalLibraryQuery m_Query;
        MetadataIO::MetadataIOService &m_MetadataIOService;
        std::vector<std::shared_ptr<SuggestionArtwork> > m_Suggestions;
        bool m_IsEnabled;
    };
}

#endif // LOCALLIBRARYQUERYENGINE_H
