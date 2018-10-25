/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UPDATEBUNDLECLEANUPJOBITEM_H
#define UPDATEBUNDLECLEANUPJOBITEM_H

#include <QString>

#include "Services/Maintenance/imaintenanceitem.h"

namespace Maintenance {
    class UpdateBundleCleanupJobItem: public IMaintenanceItem
    {
    public:
        UpdateBundleCleanupJobItem(const QString &updatesDirectoryPath);

    public:
        virtual void processJob() override;

    private:
        QString m_UpdatesDirectoryPath;
    };
}

#endif // UPDATEBUNDLECLEANUPJOBITEM_H
