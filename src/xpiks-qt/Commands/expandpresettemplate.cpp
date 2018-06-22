/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "expandpresettemplate.h"
#include "../KeywordsPresets/ipresetsmanager.h"
#include "../Artworks/artworkssnapshot.h"

namespace Commands {
    ExpandPresetTemplate::ExpandPresetTemplate(KeywordsPresets::IPresetsManager &presetsManager,
                                               KeywordsPresets::ID_t presetID,
                                               int keywordIndex):
        m_PresetsManager(presetsManager),
        m_PresetID(presetID),
        m_KeywordIndex(keywordIndex)
    {
    }

    void ExpandPresetTemplate::execute(Artworks::ArtworksSnapshot &snapshot) {
        Q_ASSERT(snapshot.size() == 1);
        QStringList keywords;

        if (m_PresetsManager.tryGetPreset(m_PresetID, keywords)) {
            Artworks::ArtworkMetadata *artwork = snapshot.get(0);
            m_ArtworksBackups.emplace_back(artwork);

            if (m_KeywordIndex != -1) {
                artwork->expandPreset(m_KeywordIndex, keywords);
            } else {
                artwork->appendKeywords(keywords);
            }
        }
    }

    void ExpandPresetTemplate::undo(Artworks::ArtworksSnapshot &snapshot) {
        Q_ASSERT(snapshot.size() == m_ArtworksBackups.size());
        Q_ASSERT(snapshot.size() == 1);
        LOG_DEBUG << "#";
        const size_t size = snapshot.size();
        for (size_t i = 0; i < size; i++) {
            Artworks::ArtworkMetadata *artwork = snapshot.get(i);
            auto &backup = m_ArtworksBackups.at(i);
            backup.restore(artwork);
        }
    }
}
