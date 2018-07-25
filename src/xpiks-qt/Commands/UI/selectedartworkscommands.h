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

#include <memory>
#include <Commands/Base/iuicommandtemplate.h>
#include <QMLExtensions/uicommandid.h>

namespace Artworks {
    class ISelectedArtworksSource;
    class ISelectedIndicesSource;
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
    class ArtworksListModel;
    class FilteredArtworksListModel;
}

namespace Commands {
    class ICommand;

    class FixSpellingInSelectedCommand: public IUICommandTemplate {
    public:
        FixSpellingInSelectedCommand(Artworks::ISelectedArtworksSource &selectedArtworksSource,
                                     SpellCheck::SpellCheckSuggestionModel spellSuggestionModel):
            m_SelectedArtworksSource(selectedArtworksSource),
            m_SpellCheckSuggestionModel(spellSuggestionModel)
        {}

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return QMLExtensions::UICommandID::FixSpellingInSelected; }
        virtual void execute(const QJSValue &) override;

    private:
        Artworks::ISelectedArtworksSource &m_SelectedArtworksSource;
        SpellCheck::SpellCheckSuggestionModel &m_SpellCheckSuggestionModel;
    };

    class ShowDuplicatesInSelectedCommand: public IUICommandTemplate {
    public:
        ShowDuplicatesInSelectedCommand(Artworks::ISelectedArtworksSource &selectedArtworksSource,
                                        SpellCheck::DuplicatesReviewModel duplicatesReviewModel):
            m_SelectedArtworksSource(selectedArtworksSource),
            m_DuplicatesReviewModel(duplicatesReviewModel)
        {}

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return QMLExtensions::UICommandID::ReviewDuplicatesInSelected; }
        virtual void execute(const QJSValue &) override;

    private:
        Artworks::ISelectedArtworksSource &m_SelectedArtworksSource;
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

    class WipeMetadataInSelectedCommand: public IUICommandTemplate {
    public:
        WipeMetadataInSelectedCommand(Artworks::ISelectedArtworksSource &selectedArtworksSource,
                                      MetadataIO::MetadataIOCoordinator &metadataIOCoordinator):
            m_SelectedArtworksSource(selectedArtworksSource),
            m_MetadataIOCoordinator(metadataIOCoordinator)
        {}

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return QMLExtensions::UICommandID::WipeMetadataInSelected; }
        virtual void execute(const QJSValue &value) override;

    private:
        Artworks::ISelectedArtworksSource &m_SelectedArtworksSource;
        MetadataIO::MetadataIOCoordinator &m_MetadataIOCoordinator;
    };

    class RemoveSelectedCommand: public IUICommandTemplate {
    public:
        RemoveSelectedCommand(Artworks::ISelectedIndicesSource &selectedIndicesSource,
                              Models::ArtworksListModel &artworksListModel,
                              const std::shared_ptr<ICommand> &saveSessionCommand):
            m_SelectedIndicesSource(selectedIndicesSource),
            m_ArtworksListModel(artworksListModel),
            m_SaveSessionCommand(saveSessionCommand)
        {}

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return QMLExtensions::UICommandID::RemoveSelected; }
        virtual void execute(const QJSValue &) override;
        virtual void undo() override;
        virtual bool canUndo() override { return true; }
        virtual QString getDescription() const override;

    private:
        Artworks::ISelectedIndicesSource &m_SelectedIndicesSource;
        Models::ArtworksListModel &m_ArtworksListModel;
        std::shared_ptr<ICommand> m_SaveSessionCommand;
    };

    class ReimportMetadataForSelected: public IUICommandTemplate {
    public:
        ReimportMetadataForSelected(Artworks::ISelectedArtworksSource &selectedArtworksSource,
                                    MetadataIO::MetadataIOCoordinator &metadataIOCoordinator):
            m_SelectedArtworksSource(selectedArtworksSource),
            m_MetadataIOCoordinator(metadataIOCoordinator)
        {}

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return QMLExtensions::UICommandID::ReimportFromSelected; }
        virtual void execute(const QJSValue &) override;

    private:
        Artworks::ISelectedArtworksSource &m_SelectedArtworksSource;
        MetadataIO::MetadataIOCoordinator &m_MetadataIOCoordinator;
    };
}

#endif // SELECTEDARTWORKSCOMMANDS_H
