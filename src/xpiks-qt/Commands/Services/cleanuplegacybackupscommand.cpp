/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cleanuplegacybackupscommand.h"

#include <vector>

#include <QFileInfo>
#include <QSet>
#include <QString>
#include <QtDebug>

#include "Common/logging.h"
#include "Filesystem/ifilescollection.h"
#include "Services/Maintenance/maintenanceservice.h"

namespace Commands {
    CleanupLegacyBackupsCommand::CleanupLegacyBackupsCommand(const std::shared_ptr<Filesystem::IFilesCollection> &files,
                                                             Maintenance::MaintenanceService &maintenanceService):
        m_MaintenanceService(maintenanceService)
    {
        QSet<QString> dirs;
        for (auto &f: files->getFiles()) {
            dirs.insert(QFileInfo(f.m_Path).absolutePath());
        }

        m_Directories = dirs.toList();
    }

    void CleanupLegacyBackupsCommand::execute() {
        LOG_DEBUG << "#";
        for (auto &dir: m_Directories) {
            m_MaintenanceService.cleanupOldXpksBackups(dir);
        }
    }
}
