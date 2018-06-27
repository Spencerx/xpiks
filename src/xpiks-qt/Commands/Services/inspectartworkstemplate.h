/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SPELLCHECKARTWORKSTEMPLATE_H
#define SPELLCHECKARTWORKSTEMPLATE_H

#include "icommandtemplate.h"

namespace SpellCheck {
    class SpellCheckerService;
}

namespace Warnings {
    class WarningsService;
}

namespace Models {
    class SettingsModel;
}

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Commands {
    class InspectArtworksTemplate: public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        InspectArtworksTemplate(SpellCheck::SpellCheckerService &spellCheckService,
                                Warnings::WarningsService &warningsService,
                                Models::SettingsModel &settingsModel);

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(const Artworks::ArtworksSnapshot &snapshot) override;
        virtual void undo(const Artworks::ArtworksSnapshot &snapshot) override { execute(snapshot); }

    private:
        SpellCheck::SpellCheckerService &m_SpellCheckService;
        Warnings::WarningsService &m_WarningsService;
        Models::SettingsModel &m_SettingsModel;
    };
}

#endif // SPELLCHECKARTWORKSTEMPLATE_H
