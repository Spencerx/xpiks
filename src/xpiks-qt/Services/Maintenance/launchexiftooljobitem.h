/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#ifndef LAUNCHEXIFTOOLJOBITEM_H
#define LAUNCHEXIFTOOLJOBITEM_H

#include <QObject>
#include <QString>
#include "imaintenanceitem.h"

namespace MetadataIO {
    class MetadataIOCoordinator;
}

namespace Maintenance {
    class LaunchExiftoolJobItem : public QObject, public IMaintenanceItem
    {
        Q_OBJECT
    public:
        LaunchExiftoolJobItem(const QString &settingsExiftoolPath);

    public:
        virtual void processJob() override;

    signals:
        void exiftoolDetected(const QString &path);

    private:
        const QString m_SettingsExiftoolPath;
        MetadataIO::MetadataIOCoordinator *m_MetadataIOCoordinator;
    };
}

#endif // LAUNCHEXIFTOOLJOBITEM_H
