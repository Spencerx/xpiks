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
#include <KeywordsPresets/ipresetsmanager.h>
#include <Artworks/artworkssnapshot.h>

namespace Commands {
    ExpandPresetTemplate::ExpandPresetTemplate(KeywordsPresets::IPresetsManager &presetsManager,
                                               KeywordsPresets::ID_t presetID,
                                               int keywordIndex):
        m_PresetsManager(presetsManager),
        m_PresetID(presetID),
        m_KeywordIndex(keywordIndex)
    {
    }

    void ExpandPresetTemplate::execute(Artworks::ArtworksSnapshot const &snapshot) {
        LOG_DEBUG << "#";
        expandPreset(snapshot);
    }

    void ExpandPresetTemplate::undo(const Artworks::ArtworksSnapshot &snapshot) {
        Q_ASSERT(snapshot.size() >= m_ArtworksBackups.size());
        Q_ASSERT(snapshot.size() == 1);
        LOG_DEBUG << "#";
        const size_t size = m_ArtworksBackups.size();
        for (size_t i = 0; i < size; i++) {
            auto &backup = m_ArtworksBackups.at(i);
            auto &artwork = snapshot.get(i);
            backup.restore(artwork);
        }
    }

    bool ExpandPresetTemplate::expandPreset(const Artworks::ArtworksSnapshot &snapshot) {
        Q_ASSERT(snapshot.size() == 1);
        QStringList keywords;

        bool found = m_PresetsManager.tryGetPreset(m_PresetID, keywords);
        if (found) {
            auto &artwork = snapshot.get(0);
            m_ArtworksBackups.emplace_back(artwork);

            if (m_KeywordIndex != -1) {
                artwork->expandPreset(m_KeywordIndex, keywords);
            } else {
                artwork->appendKeywords(keywords);
            }
        } else {
            LOG_INFO << "Not found preset" << m_PresetID;
        }

        return found;
    }

    ExpandCompletionPreset::ExpandCompletionPreset(int completionID,
                                                   KeywordsPresets::IPresetsManager &presetsManager,
                                                   KeywordsPresets::ID_t presetID):
        ExpandPresetTemplate(presetsManager, presetID),
        m_CompletionID(completionID)
    {
    }

    void ExpandCompletionPreset::execute(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "#";
        bool accepted = expandPreset(snapshot);
        auto &artwork = snapshot.get(0);
        auto *basicModel = artwork->getBasicModel();
        basicModel->notifyCompletionAccepted(accepted, m_CompletionID);
    }
}
