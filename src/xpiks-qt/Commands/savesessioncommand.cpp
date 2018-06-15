/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "savesessioncommand.h"
#include "../Models/artitemsmodel.h"
#include "../Maintenance/maintenanceservice.h"
#include "../Models/sessionmanager.h"

namespace Commands {
    SaveSessionCommand::SaveSessionCommand(Maintenance::MaintenanceService &maintenanceService,
                                           Models::ArtItemsModel &artItemsModel,
                                           Models::SessionManager &sessionManager):
        m_MaintenanceService(maintenanceService),
        m_ArtItemsModel(artItemsModel),
        m_SessionManager(sessionManager)
    {
    }

    std::shared_ptr<CommandResult> SaveSessionCommand::execute() {
        m_MaintenanceService.saveSession(m_ArtItemsModel.snapshotAll(), &m_SessionManager);
        return ICommand::execute();
    }
}
