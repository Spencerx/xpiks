/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef EXPANDPRESETTEMPLATE_H
#define EXPANDPRESETTEMPLATE_H

#include "icommandtemplate.h"
#include "../KeywordsPresets/ipresetsmanager.h"
#include "../UndoRedo/artworkmetadatabackup.h"

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Commands {
    class ExpandPresetTemplate: public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        ExpandPresetTemplate(KeywordsPresets::IPresetsManager &presetsManager,
                             KeywordsPresets::ID_t presetID,
                             int keywordIndex = -1);

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(const Artworks::ArtworksSnapshot &snapshot) override;
        virtual void undo(const Artworks::ArtworksSnapshot &snapshot) override;

    private:
        std::vector<UndoRedo::ArtworkMetadataBackup> m_ArtworksBackups;
        KeywordsPresets::IPresetsManager &m_PresetsManager;
        KeywordsPresets::ID_t m_PresetID;
        int m_KeywordIndex;
    };
}

#endif // EXPANDPRESETTEMPLATE_H
