/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "findandreplacetemplate.h"

#include <cstddef>
#include <memory>

#include <QtDebug>
#include <QtGlobal>

#include "Artworks/artworkmetadata.h"
#include "Artworks/artworkssnapshot.h"
#include "Common/logging.h"
#include "UndoRedo/artworkmetadatabackup.h"

namespace Commands {
    void FindAndReplaceTemplate::execute(const Artworks::ArtworksSnapshot &snapshot) {
        const size_t size = snapshot.size();
        LOG_INFO << size << "artwork(s)";
        m_ArtworksBackups.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            auto &artwork = snapshot.get(i);
            m_ArtworksBackups.emplace_back(UndoRedo::ArtworkMetadataBackup(artwork));
            bool succeeded = artwork->replace(m_ReplaceWhat, m_ReplaceTo, m_Flags);
            LOG_FOR_TESTS << (succeeded ? "Succeeded" : "Failed");
        }
    }

    void FindAndReplaceTemplate::undo(const Artworks::ArtworksSnapshot &snapshot) {
        const size_t size = m_ArtworksBackups.size();
        LOG_INFO << size << "artwork(s)";
        Q_ASSERT(snapshot.size() >= m_ArtworksBackups.size());
        for (size_t i = 0; i < size; i++) {
            auto &backup = m_ArtworksBackups.at(i);
            auto &artwork = snapshot.get(i);
            backup.restore(artwork);
        }
    }
}
