/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SINGLEEDITABLECOMMANDS_H
#define SINGLEEDITABLECOMMANDS_H

#include <Commands/Base/iuicommandtemplate.h>
#include <QMLExtensions/uicommandid.h>
#include "sourcetargetcommand.h"

namespace Models {
    class CombinedArtworksModel;
    class ArtworkProxyModel;
    class FilteredArtworksListModel;
    class ArtworksListModel;
}

namespace SpellCheck {
    class SpellCheckSuggestionModel;
    class DuplicatesReviewModel;
}

namespace Commands {
    namespace UI {
        SOURCE_TARGET_COMMAND(FixSpellingInCombinedEditCommand,
                              QMLExtensions::UICommandID::FixSpellingCombined,
                              Models::CombinedArtworksModel,
                              SpellCheck::SpellCheckSuggestionModel);

        SOURCE_TARGET_COMMAND(FixSpellingInArtworkProxyCommand,
                              QMLExtensions::UICommandID::FixSpellingSingle,
                              Models::ArtworkProxyModel,
                              SpellCheck::SpellCheckSuggestionModel);

        class FixSpellingInArtworkCommand: public IUICommandTemplate {
        public:
            FixSpellingInArtworkCommand(Models::FilteredArtworksListModel &filteredArtworksModel,
                                        Models::ArtworksListModel &artworksListModel,
                                        SpellCheck::SpellCheckSuggestionModel &suggestionsModel):
                m_FilteredArtworksModel(filteredArtworksModel),
                m_ArtworksListModel(artworksListModel),
                m_SuggestionsModel(suggestionsModel)
            {}

            // IUICommandTemplate interface
        public:
            virtual int getCommandID() override { return QMLExtensions::UICommandID::FixSpellingArtwork; }
            virtual void execute(const QJSValue &value) override;

        private:
            Models::FilteredArtworksListModel &m_FilteredArtworksModel;
            Models::ArtworksListModel &m_ArtworksListModel;
            SpellCheck::SpellCheckSuggestionModel &m_SuggestionsModel;
        };

        SOURCE_TARGET_COMMAND(ShowDuplicatesForSingle,
                              QMLExtensions::UICommandID::ShowDuplicatesSingle,
                              Models::ArtworkProxyModel,
                              SpellCheck::DuplicatesReviewModel);

        SOURCE_TARGET_COMMAND(ShowDuplicatesForCombined,
                              QMLExtensions::UICommandID::ShowDuplicatesCombined,
                              Models::CombinedArtworksModel,
                              SpellCheck::DuplicatesReviewModel);
    }
}

#endif // SINGLEEDITABLECOMMANDS_H
