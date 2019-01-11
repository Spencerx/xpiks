/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "addfilescommand.h"

#include <QtDebug>

#include "Artworks/artworkssnapshot.h"
#include "Commands/Base/icommandtemplate.h"
#include "Common/flags.h"
#include "Common/logging.h"
#include "Helpers/indicesranges.h"
#include "Models/Artworks/artworkslistmodel.h"
#include "Models/Artworks/artworkslistoperations.h"

namespace Commands {
    AddFilesCommand::AddFilesCommand(std::shared_ptr<Filesystem::IFilesCollection> const &files,
                                     Common::AddFilesFlags flags,
                                     Models::ArtworksListModel &artworksListModel,
                                     ArtworksCommandTemplate const &addedArtworksTemplate):
        QObject(),
        m_Files(files),
        m_Flags(flags),
        m_ArtworksListModel(artworksListModel),
        m_AddedArtworksTemplate(addedArtworksTemplate)
    { }

    void AddFilesCommand::execute() {
        LOG_DEBUG << "#";
        m_OriginalCount = m_ArtworksListModel.getArtworksSize();

        auto addResult = m_ArtworksListModel.addFiles(m_Files, m_Flags);
        m_AddedCount = addResult.m_Snapshot.size();

        if (m_AddedArtworksTemplate) {
            m_AddedArtworksTemplate->execute(addResult.m_Snapshot);
        }

        emit artworksAdded((int)addResult.m_Snapshot.size(), (int)addResult.m_AttachedVectorsCount);

        // clean resources if this will be stored in undo manager
        m_Files.reset();
    }

    void AddFilesCommand::undo() {
        LOG_DEBUG << "#";
        m_ArtworksListModel.removeFiles(Helpers::IndicesRanges((int)m_OriginalCount, (int)m_AddedCount));
    }
}
