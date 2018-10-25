/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CLEARACTIONMODELTEMPLATE_H
#define CLEARACTIONMODELTEMPLATE_H

#include "Commands/Base/icommand.h"
#include "Commands/Base/icommandtemplate.h"

namespace Models {
    class IActionModel;
}

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Commands {
    class ClearActionModelTemplate: public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        ClearActionModelTemplate(Models::IActionModel &model);

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(const Artworks::ArtworksSnapshot &) override;

    private:
        Models::IActionModel &m_Model;
    };

    class ClearActionModelCommand: public ICommand {
    public:
        ClearActionModelCommand(Models::IActionModel &model);

        // ICommand interface
    public:
        virtual void execute() override;

    private:
        Models::IActionModel &m_Model;
    };
}

#endif // CLEARACTIONMODELTEMPLATE_H
