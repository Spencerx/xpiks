/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ISUGGESTIONENGINE_H
#define ISUGGESTIONENGINE_H

#include <QString>

#include "Suggestion/isuggestionsrepository.h"

namespace Microstocks {
    class SearchQuery;
}

namespace Suggestion {
    class ISuggestionEngine: public ISuggestionsRepository {
    public:
        virtual ~ISuggestionEngine() {}

        virtual QString getName() const = 0;
        virtual int getID() const = 0;
        virtual int getMaxResultsPerPage() const = 0;
        virtual bool getIsEnabled() const = 0;
        virtual void setIsEnabled(bool value) = 0;
        virtual void submitQuery(const Microstocks::SearchQuery &query) = 0;
        virtual void cancelQuery() = 0;
    };
}

#endif // ISUGGESTIONENGINE_H
