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

namespace Models {
    class CombinedArtworksModel;
    class ArtworkProxyModel;
    class FilteredArtworksListModel;
    class ArtworksListModel;
}

namespace SpellCheck {
    class SpellCheckSuggestionModel;
    class FilteredArtworksListModel;
}

namespace Commands {
    class FixSpellingInCombinedEditCommand: public IUICommandTemplate {
    public:
        FixSpellingInCombinedEditCommand(Models::CombinedArtworksModel &combinedArtworksModel,
                                         SpellCheck::SpellCheckSuggestionModel &suggestionsModel):
            m_CombinedArtworksModel(combinedArtworksModel),
            m_SuggestionsModel(suggestionsModel)
        {}

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return QMLExtensions::UICommandID::FixSpellingCombined; }
        virtual void execute(const QJSValue &) override;

    private:
        Models::CombinedArtworksModel &m_CombinedArtworksModel;
        SpellCheck::SpellCheckSuggestionModel &m_SuggestionsModel;
    };

    class FixSpellingInArtworkProxyCommand: public IUICommandTemplate {
    public:
        FixSpellingInArtworkProxyCommand(Models::ArtworkProxyModel &artworkProxyModel,
                                         SpellCheck::SpellCheckSuggestionModel &suggestionsModel):
            m_ArtworkProxyModel(artworkProxyModel),
            m_SuggestionsModel(suggestionsModel)
        {}

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return QMLExtensions::UICommandID::FixSpellingSingle; }
        virtual void execute(const QJSValue &) override;

    private:
        Models::ArtworkProxyModel &m_ArtworkProxyModel;
        SpellCheck::SpellCheckSuggestionModel &m_SuggestionsModel;
    };

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
}

#endif // SINGLEEDITABLECOMMANDS_H
