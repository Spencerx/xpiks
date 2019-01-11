/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UPDATESCHECKERWORKER_H
#define UPDATESCHECKERWORKER_H

#include <QObject>
#include <QString>

namespace Common {
    class ISystemEnvironment;
}

namespace Models {
    class SettingsModel;
}

namespace Maintenance {
    class MaintenanceService;
}

namespace Connectivity {
    struct UpdateCheckResult {
        QString m_UpdateURL;
        QString m_Checksum;
        int m_Version;
    };

    class UpdatesCheckerWorker : public QObject
    {
        Q_OBJECT
    public:
        UpdatesCheckerWorker(Common::ISystemEnvironment &environment,
                             Models::SettingsModel &settingsModel,
                             Maintenance::MaintenanceService &maintenanceService,
                             const QString &availableUpdatePath);

    private:
        void initWorker();
        void processOneItem();
        bool checkForUpdates(UpdateCheckResult &result);
        bool downloadUpdate(const UpdateCheckResult &updateCheckResult, QString &pathToUpdate);
        bool checkAvailableUpdate(const UpdateCheckResult &updateCheckResult);

    public slots:
        void process();

    signals:
        void stopped();
        void updateAvailable(QString updateLink);
        void updateDownloaded(const QString &updatePath, int version);
        void requestFinished();
        void cancelRequested();

    private:
        Common::ISystemEnvironment &m_Environment;
        Models::SettingsModel &m_SettingsModel;
        Maintenance::MaintenanceService &m_MaintenanceService;
        QString m_UpdatesDirectory;
        QString m_AvailableUpdatePath;
    };
}

#endif // UPDATESCHECKERWORKER_H
