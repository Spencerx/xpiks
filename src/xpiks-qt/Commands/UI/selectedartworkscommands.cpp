/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "selectedartworkscommands.h"
#include <Common/logging.h>
#include <Commands/Base/icommand.h>
#include <Artworks/iselectedartworkssource.h>
#include <Artworks/iselectedindicessource.h>
#include <Artworks/artworkssnapshot.h>
#include <Services/SpellCheck/spellchecksuggestionmodel.h>
#include <Services/SpellCheck/duplicatesreviewmodel.h>
#include <Services/SpellCheck/spellsuggestionstarget.h>
#include <MetadataIO/metadataiocoordinator.h>
#include <MetadataIO/metadataioservice.h>
#include <MetadataIO/csvexportmodel.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Models/Editing/combinedartworksmodel.h>
#include <Models/Editing/findandreplacemodel.h>
#include <Models/Connectivity/artworksuploader.h>
#include <Models/Connectivity/ziparchiver.h>

namespace Commands {
    namespace UI {
        bool convertToBool(QVariant const &value, bool defaultValue = false) {
            bool result = defaultValue;
            if (value.isValid()) {
                if (value.type() == QVariant::Bool) {
                    result = value.toBool();
                }
            }
            return result;
        }

        void SaveSelectedCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            bool useBackups = false;
            bool overwrite = false;

            if (value.isValid()) {
                auto hash = value.toHash();
                auto overwriteValue = hash.value("overwrite", QVariant(false));
                if (overwriteValue.type() == QVariant::Bool) {
                    overwrite = overwriteValue.toBool();
                }
                auto backupValue = hash.value("backup", QVariant(false));
                if (backupValue.type() == QVariant::Bool) {
                    useBackups = backupValue.toBool();
                }
            }

            Artworks::ArtworksSnapshot snapshot = std::move(m_FilteredArtworksList.getArtworksToSave(overwrite));
            m_MetadataIOService.writeArtworks(snapshot);
            m_MetadataIOCoordinator.writeMetadataExifTool(snapshot, useBackups);
        }

        void FixSpellingInSelectedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            Artworks::ArtworksSnapshot snapshot = std::move(m_Source.getSelectedArtworks());
            m_SpellSuggestionsModel.setupModel(
                        std::make_shared<SpellCheck::ArtworksSuggestionTarget>(
                            snapshot, m_SpellCheckService, m_ArtworksUpdater),
                        Common::SpellCheckFlags::All);
        }

        void EditSelectedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.resetModel();
            m_Target.setArtworks(m_Source.getSelectedArtworks());
        }

        void ShowDuplicatesInSelectedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.setupModel(m_Source.getSelectedArtworks());
        }

        void WipeMetadataInSelectedCommand::execute(QVariant const &value) {
            LOG_DEBUG << value.toString();
            bool useBackups = convertToBool(value, false);
            m_Target.wipeAllMetadataExifTool(m_Source.getSelectedArtworks(), useBackups);
        }

        void ReimportMetadataForSelectedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            auto snapshot = std::move(m_Source.getSelectedArtworks());
            for (auto &artwork: snapshot) {
                artwork->prepareForReimport();
            }

            m_Target.readMetadataExifTool(snapshot, INVALID_BATCH_ID);
        }

        void ExportSelectedToCSVCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.setArtworksToExport(m_Source.getSelectedArtworks());
        }

        void FindAndReplaceInSelectedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.findReplaceCandidates(m_Source.getSelectedArtworks());
        }

        void UploadSelectedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            auto selectedArtworks = m_Source.getSelectedArtworks();
            m_Target.setArtworks(selectedArtworks);
        }

        void ZipSelectedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.setArtworks(m_Source.getSelectedArtworks());
        }
    }
}
