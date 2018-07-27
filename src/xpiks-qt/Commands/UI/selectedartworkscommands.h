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
    class CsvExportModel;
}

namespace Models {
    class ArtworksListModel;
    class FilteredArtworksListModel;
    class FindAndReplaceModel;
}

namespace Commands {
    class ICommand;

#define SOURCE_TARGET_COMMAND(ClassName, CommandID, Source, Target) \
    class ClassName: public IUICommandTemplate {\
    public:\
        ClassName(Source &source, Target &target):\
            m_Source(source),\
            m_Target(target)\
        {}\
    public:\
        virtual int getCommandID() override { return CommandID; }\
        virtual void execute(const QJSValue &) override;\
    private:\
        Source &m_Source;\
        Target &m_Target;\
    };

    SOURCE_TARGET_COMMAND(FixSpellingInSelectedCommand,
                          QMLExtensions::UICommandID::FixSpellingInSelected,
                          Artworks::ISelectedArtworksSource,
                          SpellCheck::SpellCheckSuggestionModel)

    SOURCE_TARGET_COMMAND(ShowDuplicatesInSelectedCommand,
                          QMLExtensions::UICommandID::ReviewDuplicatesInSelected,
                          Artworks::ISelectedArtworksSource,
                          SpellCheck::DuplicatesReviewModel)

    class SaveSelectedCommand: public IUICommandTemplate {
    public:
        SaveSelectedCommand(Models::FilteredArtworksListModel &filteredArtworksList,
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

    SOURCE_TARGET_COMMAND(WipeMetadataInSelectedCommand,
                          QMLExtensions::UICommandID::WipeMetadataInSelected,
                          Artworks::ISelectedArtworksSource,
                          MetadataIO::MetadataIOCoordinator)

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

    SOURCE_TARGET_COMMAND(ReimportMetadataForSelected,
                          QMLExtensions::UICommandID::ReimportFromSelected,
                          Artworks::ISelectedArtworksSource,
                          MetadataIO::MetadataIOCoordinator)

    SOURCE_TARGET_COMMAND(ExportSelectedToCSV,
                          QMLExtensions::UICommandID::ExportSelectedToCSV,
                          Artworks::ISelectedArtworksSource,
                          MetadataIO::CsvExportModel)

    SOURCE_TARGET_COMMAND(FindAndReplaceInSelected,
                          QMLExtensions::UICommandID::FindAndReplaceInSelected,
                          Artworks::ISelectedArtworksSource,
                          Models::FindAndReplaceModel)

#undef SOURCE_TARGET_COMMAND
}

#endif // SELECTEDARTWORKSCOMMANDS_H
