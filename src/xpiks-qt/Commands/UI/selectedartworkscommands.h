/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SELECTEDARTWORKSCOMMANDS_H
#define SELECTEDARTWORKSCOMMANDS_H

#include <Commands/Base/iuicommandtemplate.h>

#include <QMLExtensions/uicommandid.h>

namespace Artworks {
    class IArtworksSource;
}

namespace SpellCheck {
    class SpellCheckSuggestionModel;
    class DuplicatesReviewModel;
}

namespace MetadataIO {
    class MetadataIOCoordinator;
    class MetadataIOService;
}

namespace Models {
    class FilteredArtworksListModel;
}

namespace Commands {
    class FixSpellingInSelectedCommand: public IUICommandTemplate {
    public:
        FixSpellingInSelectedCommand(Artworks::IArtworksSource &selectedArtworksSource,
                                     SpellCheck::SpellCheckSuggestionModel spellSuggestionModel):
            m_SelectedArtworksSource(selectedArtworksSource),
            m_SpellCheckSuggestionModel(spellSuggestionModel)
        {}

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return QMLExtensions::UICommandID::FixSpellingInSelected; }
        virtual void execute(const QJSValue &) override;

    private:
        Artworks::IArtworksSource &m_SelectedArtworksSource;
        SpellCheck::SpellCheckSuggestionModel &m_SpellCheckSuggestionModel;
    };

    class ShowDuplicatesInSelectedCommand: public IUICommandTemplate {
    public:
        ShowDuplicatesInSelectedCommand(Artworks::IArtworksSource &selectedArtworksSource,
                                        SpellCheck::DuplicatesReviewModel duplicatesReviewModel):
            m_SelectedArtworksSource(selectedArtworksSource),
            m_DuplicatesReviewModel(duplicatesReviewModel)
        {}

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return QMLExtensions::UICommandID::ReviewDuplicatesInSelected; }
        virtual void execute(const QJSValue &) override;

    private:
        Artworks::IArtworksSource &m_SelectedArtworksSource;
        SpellCheck::DuplicatesReviewModel &m_DuplicatesReviewModel;
    };

    class SaveSelectedCommand: public IUICommandTemplate {
    public:
        ShowDuplicatesInSelectedCommand(Models::FilteredArtworksListModel &filteredArtworksList,
                                        MetadataIO::MetadataIOCoordinator &metadataIOCoordinator,
                                        MetadataIO::MetadataIOService &metadataIOService):
            m_FilteredArtworksList(filteredArtworksList),
            m_MetadataIOCoordinator(metadataIOCoordinator),
            m_MetadataIOService(metadataIOService)
        {}

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return QMLExtensions::UICommandID::SaveSelected; }
        virtual void execute(const QJSValue &value) override;

    private:
        Models::FilteredArtworksListModel &m_FilteredArtworksList;
        MetadataIO::MetadataIOCoordinator &m_MetadataIOCoordinator;
        MetadataIO::MetadataIOService &m_MetadataIOService;
    };

    class WipeMetadataInselectedCommand: public IUICommandTemplate {
    public:
        WipeMetadataInselectedCommand(Artworks::IArtworksSource &selectedArtworksSource,
                                      MetadataIO::MetadataIOCoordinator &metadataIOCoordinator):
            m_SelectedArtworksSource(selectedArtworksSource),
            m_MetadataIOCoordinator(metadataIOCoordinator)
        {}

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return QMLExtensions::UICommandID::WipeMetadataInSelected; }
        virtual void execute(const QJSValue &value) override;

    private:
        Artworks::IArtworksSource &m_SelectedArtworksSource;
        MetadataIO::MetadataIOCoordinator &m_MetadataIOCoordinator;
    };
}

#endif // SELECTEDARTWORKSCOMMANDS_H
