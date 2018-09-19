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

#include <QObject>
#include <memory>
#include <Commands/Base/iuicommandtemplate.h>
#include <QMLExtensions/uicommandid.h>
#include <Models/Artworks/artworkslistoperations.h>
#include "sourcetargetcommand.h"

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
    class ArtworksUploader;
    class CombinedArtworksModel;
    class ZipArchiver;
    class DeleteKeywordsViewModel;
}

namespace Services {
    class IArtworksUpdater;
}

namespace SpellCheck {
    class ISpellCheckService;
}

namespace Warnings {
    class WarningsModel;
}

namespace Commands {
    class ICommand;

    namespace UI {
        class SaveSelectedCommand: public IUICommandTemplate {
        public:
            SaveSelectedCommand(Models::FilteredArtworksListModel &filteredArtworksList,
                                MetadataIO::MetadataIOCoordinator &metadataIOCoordinator,
                                MetadataIO::MetadataIOService &metadataIOService):
                m_FilteredArtworksList(filteredArtworksList),
                m_MetadataIOCoordinator(metadataIOCoordinator),
                m_MetadataIOService(metadataIOService)
            { }

            // IUICommandTemplate interface
        public:
            virtual int getCommandID() override { return QMLExtensions::UICommandID::SaveSelected; }
            virtual void execute(QVariant const &value) override;

        private:
            Models::FilteredArtworksListModel &m_FilteredArtworksList;
            MetadataIO::MetadataIOCoordinator &m_MetadataIOCoordinator;
            MetadataIO::MetadataIOService &m_MetadataIOService;
        };

        FIX_ARTWORK_SPELLING_COMMAND(FixSpellingInSelectedCommand,
                                     QMLExtensions::UICommandID::FixSpellingInSelected,
                                     Artworks::ISelectedArtworksSource);

        SOURCE_UI_TARGET_COMMAND(EditSelectedCommand,
                                 QMLExtensions::UICommandID::EditSelectedArtworks,
                                 Artworks::ISelectedArtworksSource,
                                 Models::CombinedArtworksModel);

        SOURCE_UI_TARGET_COMMAND(ShowDuplicatesInSelectedCommand,
                                 QMLExtensions::UICommandID::ShowDuplicatesInSelected,
                                 Artworks::ISelectedArtworksSource,
                                 SpellCheck::DuplicatesReviewModel);

        SOURCE_UI_TARGET_COMMAND(WipeMetadataInSelectedCommand,
                                 QMLExtensions::UICommandID::WipeMetadataInSelected,
                                 Artworks::ISelectedArtworksSource,
                                 MetadataIO::MetadataIOCoordinator);

        SOURCE_UI_TARGET_COMMAND(ReimportMetadataForSelectedCommand,
                                 QMLExtensions::UICommandID::ReimportFromSelected,
                                 Artworks::ISelectedArtworksSource,
                                 MetadataIO::MetadataIOCoordinator);

        SOURCE_UI_TARGET_COMMAND(ExportSelectedToCSVCommand,
                                 QMLExtensions::UICommandID::ExportSelectedToCSV,
                                 Artworks::ISelectedArtworksSource,
                                 MetadataIO::CsvExportModel);

        SOURCE_UI_TARGET_COMMAND(FindAndReplaceInSelectedCommand,
                                 QMLExtensions::UICommandID::FindAndReplaceInSelected,
                                 Artworks::ISelectedArtworksSource,
                                 Models::FindAndReplaceModel);

        SOURCE_UI_TARGET_COMMAND(ZipSelectedCommand,
                                 QMLExtensions::UICommandID::ZipSelected,
                                 Artworks::ISelectedArtworksSource,
                                 Models::ZipArchiver);

        SOURCE_UI_TARGET_COMMAND(DeleteKeywordsFromSelectedCommand,
                                 QMLExtensions::UICommandID::DeleteKeywordsFromSelected,
                                 Artworks::ISelectedArtworksSource,
                                 Models::DeleteKeywordsViewModel);

        class UploadSelectedCommand: public IUICommandTemplate, public IUICommandTargetSource {
        public:
            UploadSelectedCommand(Artworks::ISelectedArtworksSource &source,
                                  Models::ArtworksUploader &uploader,
                                  Warnings::WarningsModel &warningsModel):
                m_Source(source),
                m_Uploader(uploader),
                m_WarningsModel(warningsModel)
            { }
        public:
            virtual int getCommandID() override { return QMLExtensions::UICommandID::UploadSelected; }
            virtual void execute(QVariant const &value) override;
            virtual QObject *getTargetObject() override { return (QObject*)&m_Uploader; }
        private:
            Artworks::ISelectedArtworksSource &m_Source;
            Models::ArtworksUploader &m_Uploader;
            Warnings::WarningsModel &m_WarningsModel;
        };
    }
}

#endif // SELECTEDARTWORKSCOMMANDS_H
