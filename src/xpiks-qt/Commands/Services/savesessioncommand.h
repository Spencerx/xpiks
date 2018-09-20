/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SAVESESSIONCOMMAND_H
#define SAVESESSIONCOMMAND_H

#include <Commands/Base/icommand.h>
#include <Commands/Base/icommandtemplate.h>

namespace Models {
    class ArtworksListModel;
    class SessionManager;
}

namespace Maintenance {
    class MaintenanceService;
}

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Commands {
    class SaveSessionCommand: public ICommand, public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        SaveSessionCommand(Maintenance::MaintenanceService &maintenanceService,
                           Models::ArtworksListModel &artworksListModel,
                           Models::SessionManager &sessionManager);

        // ICommand interface
    public:
        virtual void execute() override;
        virtual void undo() override { execute(); }

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(const Artworks::ArtworksSnapshot &) override { execute(); }
        virtual void undo(const Artworks::ArtworksSnapshot &) override {}

    private:
        Maintenance::MaintenanceService &m_MaintenanceService;
        Models::ArtworksListModel &m_ArtworksListModel;
        Models::SessionManager &m_SessionManager;
    };
}

#endif // SAVESESSIONCOMMAND_H
