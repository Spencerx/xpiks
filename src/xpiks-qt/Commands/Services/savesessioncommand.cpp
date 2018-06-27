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
#include "../Models/artworkslistmodel.h"
#include "../Maintenance/maintenanceservice.h"
#include "../Models/sessionmanager.h"

namespace Commands {
    SaveSessionCommand::SaveSessionCommand(Maintenance::MaintenanceService &maintenanceService,
                                           Models::ArtworksListModel &artworksListModel,
                                           Models::SessionManager &sessionManager):
        m_MaintenanceService(maintenanceService),
        m_ArtworksListModel(artworksListModel),
        m_SessionManager(sessionManager)
    {
    }

    void SaveSessionCommand::execute() {
        LOG_DEBUG << "#";
        m_MaintenanceService.saveSession(ArtworksListModel.snapshotAll(), &m_SessionManager);
    }
}
