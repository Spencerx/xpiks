/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "crashdumpcleanupjobitem.h"

#include "Common/isystemenvironment.h"
#include "Common/logging.h"
#include "Helpers/constants.h"
#include "Services/Maintenance/filescleanup.h"

#define MAX_CRASHES_SIZE_MB 10
#define MAX_CRASHES_SIZE_BYTES (MAX_CRASHES_SIZE_MB * 1024 * 1024)
#define MAX_CRASHES_AGE_DAYS 30
#define MAX_CRASHES_NUMBER 20

namespace Maintenance {
    CrashDumpCleanupJobItem::CrashDumpCleanupJobItem(Common::ISystemEnvironment &environment):
        m_CrashesDir(environment.path({Constants::CRASHES_DIR}))
    { }

    void CrashDumpCleanupJobItem::processJob() {
        LOG_DEBUG << "#";

        FilesCleanup(m_CrashesDir,
                     QStringList() << "xpiks_*.dmp",
                     QSet<QString>())
            .Run(MAX_CRASHES_NUMBER, MAX_CRASHES_AGE_DAYS, MAX_CRASHES_SIZE_BYTES);
    }
}
