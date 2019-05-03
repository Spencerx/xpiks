/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CRASHDUMPCLEANUPJOBITEM_H
#define CRASHDUMPCLEANUPJOBITEM_H

#include <QString>

#include "Services/Maintenance/imaintenanceitem.h"

namespace Common {
    class ISystemEnvironment;
}

namespace Maintenance {
    class CrashDumpCleanupJobItem: public IMaintenanceItem
    {
    public:
        CrashDumpCleanupJobItem(Common::ISystemEnvironment &environment);

    public:
        virtual void processJob() override;

    private:
        QString m_CrashesDir;
    };
}

#endif // CRASHDUMPCLEANUPJOBITEM_H
