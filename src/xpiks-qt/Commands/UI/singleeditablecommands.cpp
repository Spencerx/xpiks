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
#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Services/SpellCheck/spellchecksuggestionmodel.h>
#include <Services/SpellCheck/duplicatesreviewmodel.h>

namespace Commands {
    namespace UI {
        void FixSpellingInCombinedEditCommand::execute(const QJSValue &) {
            LOG_DEBUG << "#";
            m_Target.setupItem(&m_Source.getBasicModel());
        }

        void FixSpellingInArtworkProxyCommand::execute(const QJSValue &) {
            LOG_DEBUG << "#";
            m_Target.setupArtworks(
                        Artworks::ArtworksSnapshot({m_Source.getArtwork()}));
        }

        void FixSpellingInArtworkCommand::execute(const QJSValue &value) {
            LOG_DEBUG << value.toString();
            int index = -1;
            if (value.isNumber()) {
                index = value.toInt();
            }

            int originalIndex = m_FilteredArtworksModel.getOriginalIndex(index);
            auto *artwork = m_ArtworksListModel.getArtwork(originalIndex);
            if (artwork != nullptr) {
                m_SuggestionsModel.setupArtworks(
                            Artworks::ArtworksSnapshot({artwork}));
            } else {
                LOG_WARNING << "Cannot find artwork at" << index;
            }
        }

        void ShowDuplicatesForSingle::execute(const QJSValue &) {
            LOG_DEBUG << "#";
            m_Target.setupModel(m_Source.getArtwork()->getBasicModel());
        }

        void ShowDuplicatesForCombined::execute(const QJSValue &) {
            LOG_DEBUG << "#";
            m_Target.setupModel(&m_Source.getBasicModel());
        }
    }
}
