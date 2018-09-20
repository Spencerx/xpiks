/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ISPELLSUGGESTIONSTARGET_H
#define ISPELLSUGGESTIONSTARGET_H

#include <memory>
#include <vector>
#include <Common/flags.h>

namespace Artworks {
    class ArtworksSnapshot;
}

namespace SpellCheck {
    class SpellSuggestionsItem;

    class ISpellSuggestionsTarget {
    public:
        virtual ~ISpellSuggestionsTarget() {}

        virtual std::vector<std::shared_ptr<SpellSuggestionsItem>> generateSuggestionItems(Common::SpellCheckFlags flags) = 0;
        virtual void afterReplaceCallback() = 0;
        virtual size_t size() const = 0;
    };
}

#endif // ISPELLSUGGESTIONSTARGET_H
