/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "selectedartworkscommands.h"

#include <memory>

#include <QMap>
#include <QtDebug>

#include "Artworks/artworkmetadata.h"
#include "Artworks/artworkssnapshot.h"
#include "Artworks/iselectedartworkssource.h"
#include "Common/flags.h"
#include "Common/logging.h"
#include "Helpers/uihelpers.h"
#include "MetadataIO/csvexportmodel.h"
#include "MetadataIO/metadataiocoordinator.h"
#include "MetadataIO/metadataioservice.h"
#include "Models/Artworks/artworkslistmodel.h"
#include "Models/Artworks/filteredartworkslistmodel.h"
#include "Models/Connectivity/artworksuploader.h"
#include "Models/Connectivity/ziparchiver.h"
#include "Models/Editing/combinedartworksmodel.h"
#include "Models/Editing/deletekeywordsviewmodel.h"
#include "Models/Editing/findandreplacemodel.h"
#include "Services/SpellCheck/duplicatesreviewmodel.h"
#include "Services/SpellCheck/spellchecksuggestionmodel.h"
#include "Services/SpellCheck/spellsuggestionstarget.h"
#include "Services/Warnings/warningsmodel.h"

namespace Commands {
    namespace UI {
        void SetupExportMetadataCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            bool useBackups = false;
            bool overwrite = false;

            if (value.isValid()) {
                auto map = value.toMap();
                auto overwriteValue = map.value("overwrite", QVariant(false));
                if (overwriteValue.type() == QVariant::Bool) {
                    overwrite = overwriteValue.toBool();
                }
                auto backupValue = map.value("backup", QVariant(false));
                if (backupValue.type() == QVariant::Bool) {
                    useBackups = backupValue.toBool();
                }
            }

            Artworks::ArtworksSnapshot snapshot = m_FilteredArtworksList.getArtworksToSave(overwrite);
            m_MetadataIOService.writeArtworks(snapshot);
            m_MetadataIOCoordinator.writeMetadataExifTool(snapshot, useBackups);
        }

        void ReviewSpellingInSelectedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            Artworks::ArtworksSnapshot snapshot = m_Source.getSelectedArtworks();
            m_SpellSuggestionsModel.setupModel(
                        std::make_shared<SpellCheck::ArtworksSuggestionTarget>(
                            snapshot, m_SpellCheckService, m_ArtworksUpdater),
                        Common::SpellCheckFlags::All);
        }

        void SetupEditSelectedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.resetModel();
            m_Target.setArtworks(m_Source.getSelectedArtworks());
        }

        void ReviewDuplicatesInSelectedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.setupModel(m_Source.getSelectedArtworks());
        }

        void WipeMetadataCommand::execute(QVariant const &value) {
            LOG_DEBUG << value.toString();
            bool useBackups = Helpers::convertToBool(value, false);
            m_Target.wipeAllMetadataExifTool(m_Source.getSelectedArtworks(), useBackups);
        }

        void SetupReimportMetadataCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            auto snapshot = m_Source.getSelectedArtworks();
            for (auto &artwork: snapshot) {
                artwork->prepareForReimport();
            }

            m_Target.reimportMetadataExiftool(snapshot);
        }

        void SetupCSVExportForSelectedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.setArtworksToExport(m_Source.getSelectedArtworks());
        }

        void FindReplaceCandidatesCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.findReplaceCandidates(m_Source.getFilteredArtworks());
        }

        void SetupCreatingArchivesCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.setArtworks(m_Source.getSelectedArtworks());
        }

        void SetupDeleteKeywordsInSelectedCommand::execute(const QVariant &) {
            LOG_DEBUG << "#";
            m_Target.setArtworks(m_Source.getSelectedArtworks());
        }

        void SetupUploadCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            m_Uploader.clearModel();
            bool skipUploadItems = Helpers::convertToBool(value, false);
            if (!skipUploadItems) {
                auto selectedArtworks = m_Source.getSelectedArtworks();
                m_Uploader.setArtworks(selectedArtworks);
                m_WarningsModel.setShowSelected();
            }
        }

        void UpdateSelectedArtworksCommand::execute(const QVariant &) {
            LOG_DEBUG << "#";
            update();
        }

        void UpdateSelectedArtworksCommand::undo(const QVariant &) {
            LOG_DEBUG << "#";
            update();
        }

        void UpdateSelectedArtworksCommand::update() {
            m_Source.updateSelection(Models::ArtworksListModel::SelectionType::Selected,
                                     m_Source.getStandardUpdateRoles());
        }
    }
}
