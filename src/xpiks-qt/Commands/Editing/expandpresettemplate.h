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

#include <vector>

#include "Commands/Base/icommandtemplate.h"
#include "KeywordsPresets/presetmodel.h"
#include "UndoRedo/artworkmetadatabackup.h"

namespace KeywordsPresets {
    class IPresetsManager;
}

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
        virtual void execute(Artworks::ArtworksSnapshot const &snapshot) override;
        virtual void undo(Artworks::ArtworksSnapshot const &snapshot) override;

    protected:
        bool expandPreset(Artworks::ArtworksSnapshot const &snapshot);

    private:
        std::vector<UndoRedo::ArtworkMetadataBackup> m_ArtworksBackups;
        KeywordsPresets::IPresetsManager &m_PresetsManager;
        KeywordsPresets::ID_t m_PresetID;
        int m_KeywordIndex;
    };

    class ExpandCompletionPreset: public ExpandPresetTemplate {
    public:
        ExpandCompletionPreset(int completionID,
                               KeywordsPresets::IPresetsManager &presetsManager,
                               KeywordsPresets::ID_t presetID);

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(Artworks::ArtworksSnapshot const &snapshot) override;

    private:
        int m_CompletionID;  // this value is unused - it should be removed?
    };
}

#endif // EXPANDPRESETTEMPLATE_H
