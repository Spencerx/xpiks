/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ISUGGESTIONSREPOSITORY_H
#define ISUGGESTIONSREPOSITORY_H

#include <memory>
#include <vector>

namespace Suggestion {
    class SuggestionArtwork;

    class ISuggestionsRepository {
    public:
        virtual ~ISuggestionsRepository() {}
        virtual void setSuggestions(std::vector<std::shared_ptr<SuggestionArtwork> > &suggestions) = 0;
        virtual std::vector<std::shared_ptr<SuggestionArtwork> > &getSuggestions() = 0;
        virtual void setError() = 0;
    };
}

#endif // ISUGGESTIONSREPOSITORY_H
