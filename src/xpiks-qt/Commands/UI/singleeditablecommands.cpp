/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "singleeditablecommands.h"
#include <Models/Editing/combinedartworksmodel.h>
#include <Models/Editing/artworkproxymodel.h>
#include <Services/SpellCheck/spellchecksuggestionmodel.h>

namespace Commands {
    void FixSpellingInCombinedEditCommand::execute(const QJSValue &) {
        LOG_DEBUG << "#";
        m_SuggestionsModel.setupItem(&m_CombinedArtworksModel.getBasicModel());
    }

    void FixSpellingInArtworkProxyCommand::execute(const QJSValue &) {
        LOG_DEBUG << "#";
        m_SuggestionsModel.setupArtworks(
                    Artworks::ArtworksSnapshot({m_ArtworkProxyModel.getArtwork()}));
    }
}
