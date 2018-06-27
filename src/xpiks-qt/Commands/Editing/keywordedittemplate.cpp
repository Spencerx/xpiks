/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "keywordedittemplate.h"
#include "../Artworks/artworkssnapshot.h"
#include "../Common/logging.h"

namespace Commands {
    KeywordEditTemplate::KeywordEditTemplate(Common::KeywordEditFlags editFlags,
                                             int keywordIndex,
                                             const QString &nextValue):
        m_KeywordIndex(keywordIndex),
        m_EditFlags(editFlags),
        m_NextValue(nextValue)
    {
    }

    void KeywordEditTemplate::execute(const Artworks::ArtworksSnapshot &snapshot) {
        Q_ASSERT(snapshot.size() == 1);
        LOG_INFO << "artworks count =" << snapshot.size();
        const size_t size = snapshot.size();
        m_ArtworksBackups.reserve(size);
        QString removed;

        for (size_t i = 0; i < size; ++i) {
            Artworks::ArtworkMetadata *artwork = snapshot.get(i);

            m_ArtworksBackups.emplace_back(UndoRedo::ArtworkMetadataBackup(artwork));

            switch (m_EditFlags) {
            case Common::KeywordEditFlags::Remove:
                artwork->removeKeywordAt(m_KeywordIndex, removed);
                break;
            case Common::KeywordEditFlags::RemoveLast:
                artwork->removeLastKeyword(removed);
                break;
            case Common::KeywordEditFlags::Replace:
                artwork->editKeyword(m_KeywordIndex, m_NextValue);
                break;
            }
        }
    }

    void KeywordEditTemplate::undo(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "#";
        Q_ASSERT(snapshot.size() >= m_ArtworksBackups.size());
        const size_t size = m_ArtworksBackups.size();
        for (size_t i = 0; i < size; i++) {
            auto &backup = m_ArtworksBackups.at(i);
            Artworks::ArtworkMetadata *artwork = snapshot.get(i);
            backup.restore(artwork);
        }
    }
}
