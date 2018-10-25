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

#include <QVariant>

#include "Commands/Base/iuicommandtemplate.h"
#include "Commands/UI/sourcetargetcommand.h"
#include "QMLExtensions/uicommandid.h"

class QObject;

namespace Models {
    class ArtworkProxyModel;
    class CombinedArtworksModel;
    class FilteredArtworksListModel;
    class QuickBuffer;
}

namespace SpellCheck {
    class DuplicatesReviewModel;
    class ISpellCheckService;
    class SpellCheckSuggestionModel;
}

namespace AutoComplete {
    class AutoCompleteService;
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
    class ICommandManager;
}

namespace Commands {
    namespace UI {
        class ReviewSpellingInBasicModelCommand: public IUICommandTemplate {
        public:
            ReviewSpellingInBasicModelCommand(QMLExtensions::UICommandID::CommandID commandID,
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

        FIX_ARTWORK_SPELLING_COMMAND(ReviewSpellingInArtworkProxyCommand,
                                     QMLExtensions::UICommandID::ReviewSpellingSingle,
                                     Models::ArtworkProxyModel);

        FIX_ARTWORK_SPELLING_COMMAND(ReviewSpellingInArtworkCommand,
                                     QMLExtensions::UICommandID::ReviewSpellingArtwork,
                                     Models::FilteredArtworksListModel);

        SOURCE_UI_TARGET_COMMAND(ReviewDuplicatesForSingleCommand,
                                 QMLExtensions::UICommandID::ReviewDuplicatesSingle,
                                 Models::ArtworkProxyModel,
                                 SpellCheck::DuplicatesReviewModel);

        SOURCE_UI_TARGET_COMMAND(ReviewDuplicatesForCombinedCommand,
                                 QMLExtensions::UICommandID::ReviewDuplicatesCombined,
                                 Models::CombinedArtworksModel,
                                 SpellCheck::DuplicatesReviewModel);

        SOURCE_UI_TARGET_COMMAND(ReviewDuplicatesForArtworkCommand,
                                 QMLExtensions::UICommandID::ReviewDuplicatesArtwork,
                                 Models::FilteredArtworksListModel,
                                 SpellCheck::DuplicatesReviewModel);

        SOURCE_UI_TARGET_COMMAND(AcceptPresetCompletionForCombinedCommand,
                                 QMLExtensions::UICommandID::AcceptPresetCompletionForCombined,
                                 AutoComplete::ICompletionSource,
                                 Models::CombinedArtworksModel);

        SOURCE_UI_TARGET_COMMAND(AcceptPresetCompletionForSingleCommand,
                                 QMLExtensions::UICommandID::AcceptPresetCompletionForSingle,
                                 AutoComplete::ICompletionSource,
                                 Models::ArtworkProxyModel);

        SOURCE_UI_TARGET_COMMAND(AcceptPresetCompletionForArtworkCommand,
                                 QMLExtensions::UICommandID::AcceptPresetCompletionForArtwork,
                                 AutoComplete::ICompletionSource,
                                 Models::FilteredArtworksListModel);

        SOURCE_TARGET_COMMAND(GenerateCompletionsForArtworkCommand,
                              QMLExtensions::UICommandID::GenerateCompletionsForArtwork,
                              Models::FilteredArtworksListModel,
                              AutoComplete::AutoCompleteService);

        SOURCE_UI_TARGET_COMMAND(InitSuggestionForArtworkCommand,
                                 QMLExtensions::UICommandID::InitSuggestionArtwork,
                                 Models::FilteredArtworksListModel,
                                 Suggestion::KeywordsSuggestor);

        SOURCE_UI_TARGET_COMMAND(InitSuggestionForCombinedCommand,
                                 QMLExtensions::UICommandID::InitSuggestionCombined,
                                 Models::CombinedArtworksModel,
                                 Suggestion::KeywordsSuggestor);

        SOURCE_UI_TARGET_COMMAND(InitSuggestionForSingleCommand,
                                 QMLExtensions::UICommandID::InitSuggestionSingle,
                                 Models::ArtworkProxyModel,
                                 Suggestion::KeywordsSuggestor);

        SOURCE_UI_TARGET_COMMAND(SetupArtworkEditCommand,
                                 QMLExtensions::UICommandID::SetupProxyArtworkEdit,
                                 Models::FilteredArtworksListModel,
                                 Models::ArtworkProxyModel);

        class ReviewArtworkInfoCommand: public SetupArtworkEditCommand {
        public:
            ReviewArtworkInfoCommand(Models::FilteredArtworksListModel &source,
                                   Models::ArtworkProxyModel &target):
                SetupArtworkEditCommand(source, target)
            {}
        public:
            virtual int getCommandID() override { return QMLExtensions::UICommandID::ReviewArtworkInfo; }
        };

        SOURCE_COMMAND(CopyArtworkToQuickBufferCommand,
                       QMLExtensions::UICommandID::CopyArtworkToQuickBuffer,
                       Models::FilteredArtworksListModel);

        SOURCE_COMMAND(CopyCombinedToQuickBufferCommand,
                       QMLExtensions::UICommandID::CopyCombinedToQuickBuffer,
                       Models::CombinedArtworksModel);

        class FillArtworkFromQuickBufferCommand: public IUICommandTemplate {
        public:
            FillArtworkFromQuickBufferCommand(Models::QuickBuffer &quickBuffer,
                                              Models::FilteredArtworksListModel &filteredArtworksList,
                                              Commands::ICommandManager &commandManager,
                                              Services::IArtworksUpdater &updater):
                m_QuickBuffer(quickBuffer),
                m_FilteredArtworksList(filteredArtworksList),
                m_CommandManager(commandManager),
                m_Updater(updater)
            { }

            // IUICommandTemplate interface
        public:
            virtual int getCommandID() override { return QMLExtensions::UICommandID::FillArtworkFromQuickBuffer; }
            virtual void execute(const QVariant &value) override;

        private:
            Models::QuickBuffer &m_QuickBuffer;
            Models::FilteredArtworksListModel &m_FilteredArtworksList;
            Commands::ICommandManager &m_CommandManager;
            Services::IArtworksUpdater &m_Updater;
        };
    }
}

#endif // SINGLEEDITABLECOMMANDS_H
