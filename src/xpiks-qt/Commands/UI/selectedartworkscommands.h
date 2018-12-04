﻿/*
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

#include <QVariant>

#include "Commands/Base/iuicommandtemplate.h"
#include "Commands/UI/sourcetargetcommand.h"
#include "QMLExtensions/uicommandid.h"

class QObject;

namespace Artworks {
    class ISelectedArtworksSource;
}

namespace SpellCheck {
    class DuplicatesReviewModel;
}

namespace MetadataIO {
    class CsvExportModel;
    class MetadataIOCoordinator;
    class MetadataIOService;
}

namespace Models {
    class ArtworksListModel;
    class ArtworksUploader;
    class CombinedArtworksModel;
    class DeleteKeywordsViewModel;
    class FilteredArtworksListModel;
    class FindAndReplaceModel;
    class ZipArchiver;
}

namespace Warnings {
    class WarningsModel;
}

namespace Commands {

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

        SOURCE_UI_TARGET_COMMAND(FindReplaceCandidatesCommand,
                                 QMLExtensions::UICommandID::FindReplaceCandidates,
                                 Models::FilteredArtworksListModel,
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

        class UpdateSelectedArtworksCommand: public IUICommandTemplate {
        public:
            UpdateSelectedArtworksCommand(Models::ArtworksListModel &source):
                m_Source(source)
            { }
        public:
            virtual int getCommandID() override { return QMLExtensions::UICommandID::UpdateSelected; }
            virtual void execute(QVariant const &value) override;
            virtual void undo(const QVariant &) override;
        private:
            void update();
        private:
            Models::ArtworksListModel &m_Source;
        };
    }
}

#endif // SELECTEDARTWORKSCOMMANDS_H
