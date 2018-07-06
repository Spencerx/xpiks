/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "deletekeywordstemplate.h"
#include <Common/defines.h>
#include <Common/logging.h>
#include <Artworks/artworkmetadata.h>

namespace Commands {
    DeleteKeywordsTemplate::DeleteKeywordsTemplate(const QSet<QString> &keywordsSet, bool caseSensitive):
        m_KeywordsSet(keywordsSet),
        m_CaseSensitive(caseSensitive)
    {
    }

    void DeleteKeywordsTemplate::execute(const Artworks::ArtworksSnapshot &snapshot) {
        const size_t size = snapshot.size();
        m_ArtworksBackups.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            Artworks::ArtworkMetadata *artwork = snapshot.get(i);
            m_ArtworksBackups.emplace_back(UndoRedo::ArtworkMetadataBackup(artwork));
            artwork->removeKeywords(m_KeywordsSet, m_CaseSensitive);
        }
    }

    void DeleteKeywordsTemplate::undo(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "#";
        Q_ASSERT(snapshot.size() == m_ArtworksBackups.size());
        const size_t size = snapshot.size();
        for (size_t i = 0; i < size; i++) {
            Artworks::ArtworkMetadata *artwork = snapshot.get(i);
            auto &backup = m_ArtworksBackups.at(i);
            backup.restore(artwork);
        }
    }
}
