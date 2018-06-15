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

#include "icommand.h"

namespace Models {
    class ArtItemsModel;
    class SessionManager;
}

namespace Maintenance {
    class MaintenanceService;
}

namespace Commands {
    class SaveSessionCommand : public ICommand
    {
    public:
        SaveSessionCommand(Maintenance::MaintenanceService &maintenanceService,
                           Models::ArtItemsModel &artItemsModel,
                           Models::SessionManager &sessionManager);

        // ICommand interface
    public:
        virtual std::shared_ptr<CommandResult> execute() override;

    private:
        Maintenance::MaintenanceService &m_MaintenanceService;
        Models::ArtItemsModel &m_ArtItemsModel;
        Models::SessionManager &m_SessionManager;
    };
}

#endif // SAVESESSIONCOMMAND_H
