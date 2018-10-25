/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SPELLSUGGESTIONSTARGET_H
#define SPELLSUGGESTIONSTARGET_H

#include <cstddef>
#include <memory>
#include <vector>

#include "Artworks/artworkssnapshot.h"
#include "Common/flags.h"
#include "Services/SpellCheck/ispellsuggestionstarget.h"

namespace Artworks {
    class BasicMetadataModel;
}

namespace Services {
    class IArtworksUpdater;
}

namespace SpellCheck {
    class ISpellCheckService;
    class SpellSuggestionsItem;

    class BasicModelSuggestionTarget: public ISpellSuggestionsTarget {
    public:
        BasicModelSuggestionTarget(Artworks::BasicMetadataModel &basicModel, ISpellCheckService &spellCheckService);

        // ISpellSuggestionsTarget interface
    public:
        virtual std::vector<std::shared_ptr<SpellSuggestionsItem> > generateSuggestionItems(Common::SpellCheckFlags flags) override;
        virtual void afterReplaceCallback() override;
        virtual size_t size() const override { return 1; }

    private:
        Artworks::BasicMetadataModel &m_BasicModel;
        ISpellCheckService &m_SpellCheckService;
    };

    class ArtworksSuggestionTarget: public ISpellSuggestionsTarget {
    public:
        ArtworksSuggestionTarget(Artworks::ArtworksSnapshot &snapshot,
                                 ISpellCheckService &spellCheckService,
                                 Services::IArtworksUpdater &artworksUpdater);

        // ISpellSuggestionsTarget interface
    public:
        virtual std::vector<std::shared_ptr<SpellSuggestionsItem>> generateSuggestionItems(Common::SpellCheckFlags flags) override;
        virtual void afterReplaceCallback() override;
        virtual size_t size() const override;

    private:
        Artworks::ArtworksSnapshot m_Snapshot;
        ISpellCheckService &m_SpellCheckService;
        Services::IArtworksUpdater &m_ArtworksUpdater;
    };
}

#endif // SPELLSUGGESTIONSTARGET_H
