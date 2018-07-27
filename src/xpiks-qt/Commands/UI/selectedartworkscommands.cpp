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
#include <MetadataIO/metadataiocoordinator.h>
#include <MetadataIO/metadataioservice.h>
#include <MetadataIO/csvexportmodel.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Models/Editing/findandreplacemodel.h>

namespace Commands {
    void FixSpellingInSelectedCommand::execute(const QJSValue &) {
        LOG_DEBUG << "#";
        Artworks::ArtworksSnapshot snapshot = std::move(m_Source.getSelectedArtworks());
        m_Target.setupArtworks(snapshot);
    }

    void ShowDuplicatesInSelectedCommand::execute(const QJSValue &) {
        LOG_DEBUG << "#";
        m_Target.setupModel(m_Source.getSelectedArtworks());
    }

    void SaveSelectedCommand::execute(const QJSValue &value) {
        LOG_DEBUG << value;
        bool useBackups = false;
        if (value.isBool()) {
            useBackups = value.toBool();
        }

        Artworks::ArtworksSnapshot snapshot = std::move(m_FilteredArtworksList.getArtworksToSave(false));
        m_MetadataIOService.writeArtworks(snapshot);
        m_MetadataIOCoordinator.writeMetadataExifTool(snapshot, useBackups);
    }

    void WipeMetadataInSelectedCommand::execute(const QJSValue &value) {
        LOG_DEBUG << value;
        bool useBackups = false;
        if (value.isBool()) {
            useBackups = value.toBool();
        }

        m_Target.wipeAllMetadataExifTool(m_Source.getSelectedArtworks(), useBackups);
    }

    void RemoveSelectedCommand::execute(const QJSValue &) {
        LOG_DEBUG << "#";
        auto indices = m_SelectedIndicesSource.getSelectedIndices();
        auto removeResult = m_ArtworksListModel.removeFiles(Helpers::IndicesRanges(indices));
        if (removeResult.m_RemovedCount > 0) {
            m_SaveSessionCommand->execute();
        }
    }

    void ReimportMetadataForSelected::execute(const QJSValue &) {
        LOG_DEBUG << "#";
        m_Target.readMetadataExifTool(m_Source.getSelectedArtworks(), INVALID_BATCH_ID);
    }

    void ExportSelectedToCSV::execute(const QJSValue &) {
        LOG_DEBUG << "#";
        m_Target.setArtworksToExport(m_Source.getSelectedArtworks());
    }

    void FindAndReplaceInSelected::execute(const QJSValue &) {
        LOG_DEBUG << "#";
        m_Target.findReplaceCandidates(m_Source.getSelectedArtworks());
    }
}
