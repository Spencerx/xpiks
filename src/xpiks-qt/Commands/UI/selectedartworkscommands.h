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
        class SetupExportMetadataCommand: public IUICommandTemplate {
        public:
            SetupExportMetadataCommand(Models::FilteredArtworksListModel &filteredArtworksList,
                                MetadataIO::MetadataIOCoordinator &metadataIOCoordinator,
                                MetadataIO::MetadataIOService &metadataIOService):
                m_FilteredArtworksList(filteredArtworksList),
                m_MetadataIOCoordinator(metadataIOCoordinator),
                m_MetadataIOService(metadataIOService)
            { }

            // IUICommandTemplate interface
        public:
            virtual int getCommandID() override { return QMLExtensions::UICommandID::SetupExportMetadata; }
            virtual void execute(QVariant const &value) override;

        private:
            Models::FilteredArtworksListModel &m_FilteredArtworksList;
            MetadataIO::MetadataIOCoordinator &m_MetadataIOCoordinator;
            MetadataIO::MetadataIOService &m_MetadataIOService;
        };

        FIX_ARTWORK_SPELLING_COMMAND(ReviewSpellingInSelectedCommand,
                                     QMLExtensions::UICommandID::ReviewSpellingInSelected,
                                     Artworks::ISelectedArtworksSource);

        SOURCE_UI_TARGET_COMMAND(SetupEditSelectedCommand,
                                 QMLExtensions::UICommandID::SetupEditSelectedArtworks,
                                 Artworks::ISelectedArtworksSource,
                                 Models::CombinedArtworksModel);

        SOURCE_UI_TARGET_COMMAND(ReviewDuplicatesInSelectedCommand,
                                 QMLExtensions::UICommandID::ReviewDuplicatesInSelected,
                                 Artworks::ISelectedArtworksSource,
                                 SpellCheck::DuplicatesReviewModel);

        SOURCE_UI_TARGET_COMMAND(SetupWipeMetadataCommand,
                                 QMLExtensions::UICommandID::SetupWipeMetadata,
                                 Artworks::ISelectedArtworksSource,
                                 MetadataIO::MetadataIOCoordinator);

        SOURCE_UI_TARGET_COMMAND(SetupReimportMetadataCommand,
                                 QMLExtensions::UICommandID::SetupReimportMetadata,
                                 Artworks::ISelectedArtworksSource,
                                 MetadataIO::MetadataIOCoordinator);

        SOURCE_UI_TARGET_COMMAND(SetupCSVExportForSelectedCommand,
                                 QMLExtensions::UICommandID::SetupCSVExportForSelected,
                                 Artworks::ISelectedArtworksSource,
                                 MetadataIO::CsvExportModel);

        SOURCE_UI_TARGET_COMMAND(FindAndReplaceInSelectedCommand,
                                 QMLExtensions::UICommandID::FindAndReplaceInSelected,
                                 Artworks::ISelectedArtworksSource,
                                 Models::FindAndReplaceModel);

        SOURCE_UI_TARGET_COMMAND(SetupCreatingArchivesCommand,
                                 QMLExtensions::UICommandID::SetupCreatingArchives,
                                 Artworks::ISelectedArtworksSource,
                                 Models::ZipArchiver);

        SOURCE_UI_TARGET_COMMAND(SetupDeleteKeywordsInSelectedCommand,
                                 QMLExtensions::UICommandID::SetupDeleteKeywordsInSelected,
                                 Artworks::ISelectedArtworksSource,
                                 Models::DeleteKeywordsViewModel);

        class SetupUploadCommand: public IUICommandTemplate, public IUICommandTargetSource {
        public:
            SetupUploadCommand(Artworks::ISelectedArtworksSource &source,
                                  Models::ArtworksUploader &uploader,
                                  Warnings::WarningsModel &warningsModel):
                m_Source(source),
                m_Uploader(uploader),
                m_WarningsModel(warningsModel)
            { }
        public:
            virtual int getCommandID() override { return QMLExtensions::UICommandID::SetupUpload; }
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
