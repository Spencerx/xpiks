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

#include "iartworkscommandtemplate.h"

namespace SpellCheck {
    class SpellCheckerService;
}

namespace Commands {
    class InspectArtworksTemplate: public IArtworksCommandTemplate
    {
    public:
        InspectArtworksTemplate(SpellCheck::SpellCheckerService &spellCheckService);

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(Artworks::ArtworksSnapshot &snapshot) override;
        virtual void undo(Artworks::ArtworksSnapshot &) override { execute(snapshot); }

    private:
        SpellCheck::SpellCheckerService &m_SpellCheckService;
    };
}

#endif // SPELLCHECKARTWORKSTEMPLATE_H
