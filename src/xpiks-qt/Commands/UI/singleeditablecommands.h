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

namespace AutoComplete {
    class ICompletionSource;
}

namespace Suggestion {
    class KeywordsSuggestor;
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


        SOURCE_TARGET_COMMAND(FixSpellingForArtworkCommand,
                              QMLExtensions::UICommandID::FixSpellingArtwork,
                              Models::FilteredArtworksListModel,
                              SpellCheck::SpellCheckSuggestionModel);

        SOURCE_TARGET_COMMAND(ShowDuplicatesForSingleCommand,
                              QMLExtensions::UICommandID::ShowDuplicatesSingle,
                              Models::ArtworkProxyModel,
                              SpellCheck::DuplicatesReviewModel);

        SOURCE_TARGET_COMMAND(ShowDuplicatesForCombinedCommand,
                              QMLExtensions::UICommandID::ShowDuplicatesCombined,
                              Models::CombinedArtworksModel,
                              SpellCheck::DuplicatesReviewModel);

        SOURCE_TARGET_COMMAND(AcceptPresetCompletionForCombinedCommand,
                              QMLExtensions::UICommandID::AcceptPresetCompletionCombined,
                              AutoComplete::ICompletionSource,
                              Models::CombinedArtworksModel);

        SOURCE_TARGET_COMMAND(InitSuggestionForArtworkCommand,
                              QMLExtensions::UICommandID::InitSuggestionArtwork,
                              Models::FilteredArtworksListModel,
                              Suggestion::KeywordsSuggestor);

        SOURCE_TARGET_COMMAND(InitSuggestionForCombinedCommand,
                              QMLExtensions::UICommandID::InitSuggestionCombined,
                              Models::CombinedArtworksModel,
                              Suggestion::KeywordsSuggestor);

        SOURCE_TARGET_COMMAND(InitSuggestionForSingleCommand,
                              QMLExtensions::UICommandID::InitSuggestionSingle,
                              Models::ArtworkProxyModel,
                              Suggestion::KeywordsSuggestor);
    }
}

#endif // SINGLEEDITABLECOMMANDS_H
