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
    class ISpellCheckService;
    class ISpellCheckService;
}

namespace AutoComplete {
    class ICompletionSource;
}

namespace Suggestion {
    class KeywordsSuggestor;
}

namespace Artworks {
    class IBasicModelSource;
}

namespace Services {
    class IArtworksUpdater;
}

namespace Commands {
    namespace UI {
        class FixSpellingInBasicModelCommand: public IUICommandTemplate {
        public:
            FixSpellingInBasicModelCommand(QMLExtensions::UICommandID::CommandID commandID,
                                           Artworks::IBasicModelSource &basicModelSource,
                                           SpellCheck::ISpellCheckService &spellCheckService,
                                           SpellCheck::SpellCheckSuggestionModel &spellSuggestionsModel):
                m_CommandID(commandID),
                m_BasicModelSource(basicModelSource),
                m_SpellCheckService(spellCheckService),
                m_SpellSuggestionsModel(spellSuggestionsModel)
            {}

            // IUICommandTemplate interface
        public:
            virtual int getCommandID() override { return m_CommandID; }
            virtual void execute(const QVariant &value) override;

        private:
            QMLExtensions::UICommandID::CommandID m_CommandID;
            Artworks::IBasicModelSource &m_BasicModelSource;
            SpellCheck::ISpellCheckService &m_SpellCheckService;
            SpellCheck::SpellCheckSuggestionModel &m_SpellSuggestionsModel;
        };

        FIX_ARTWORK_SPELLING_COMMAND(FixSpellingInArtworkProxyCommand,
                                     QMLExtensions::UICommandID::FixSpellingSingle,
                                     Models::ArtworkProxyModel);

        FIX_ARTWORK_SPELLING_COMMAND(FixSpellingForArtworkCommand,
                                     QMLExtensions::UICommandID::FixSpellingArtwork,
                                     Models::FilteredArtworksListModel);

        SOURCE_TARGET_COMMAND(ShowDuplicatesForSingleCommand,
                              QMLExtensions::UICommandID::ShowDuplicatesSingle,
                              Models::ArtworkProxyModel,
                              SpellCheck::DuplicatesReviewModel);

        SOURCE_TARGET_COMMAND(ShowDuplicatesForCombinedCommand,
                              QMLExtensions::UICommandID::ShowDuplicatesCombined,
                              Models::CombinedArtworksModel,
                              SpellCheck::DuplicatesReviewModel);

        SOURCE_TARGET_COMMAND(AcceptPresetCompletionForCombinedCommand,
                              QMLExtensions::UICommandID::AcceptPresetCompletionForCombined,
                              AutoComplete::ICompletionSource,
                              Models::CombinedArtworksModel);

        SOURCE_TARGET_COMMAND(AcceptPresetCompletionForSingleCommand,
                              QMLExtensions::UICommandID::AcceptPresetCompletionForSingle,
                              AutoComplete::ICompletionSource,
                              Models::ArtworkProxyModel);

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
