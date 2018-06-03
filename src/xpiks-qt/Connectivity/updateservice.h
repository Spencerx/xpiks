/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UPDATESERVICE_H
#define UPDATESERVICE_H

#include <QObject>
#include "../Common/statefulentity.h"
#include "../Helpers/constants.h"
#include "../Common/isystemenvironment.h"
#include "../Common/logging.h"

namespace Models {
    class SettingsModel;
    class SwitcherModel;
}

namespace Maintenance {
    class MaintenanceService;
}

namespace Connectivity {
    class UpdatesCheckerWorker;

    class UpdateService : public QObject
    {
        Q_OBJECT
    public:
        UpdateService(Common::ISystemEnvironment &environment,
                      Models::SettingsModel *settingsModel,
                      Models::SwitcherModel *switcherModel,
                      Maintenance::MaintenanceService *maintenanceService);

    public:
        void initialize();

    public:
        void startChecking();
        void stopChecking();

    public:
        void setHaveUpgradeConsent() { m_HaveUpgradeConsent = true; }
        void tryToUpgradeXpiks();
        bool getIsUpdateDownloaded() { return !getPathToUpdate().isEmpty(); }

    private:
        void doStartChecking(const QString &pathToUpdate);
        void updateSettings();

    private:
        int getAvailableUpdateVersion() const { return m_State.getInt(Constants::availableUpdateVersion); }
        QString getPathToUpdate() const { return m_State.getString(Constants::pathToUpdate); }

        void setAvailableUpdateVersion(int version) {
            LOG_DEBUG << "#";
            m_State.setValue(Constants::availableUpdateVersion, version);
            m_State.sync();
        }

        void setPathToUpdate(QString path) {
            LOG_DEBUG << "#";
            m_State.setValue(Constants::pathToUpdate, path);
            m_State.sync();
        }

    private slots:
        void workerFinished();
        void updateDownloadedHandler(const QString &updatePath, int version);

    signals:
        void updateAvailable(QString updateLink);
        void updateDownloaded();
        void cancelRequested();

    private:
        Common::ISystemEnvironment &m_Environment;
        Connectivity::UpdatesCheckerWorker *m_UpdatesCheckerWorker;
        Models::SettingsModel *m_SettingsModel;
        Models::SwitcherModel *m_SwitcherModel;
        Maintenance::MaintenanceService *m_MaintenanceService;
        Common::StatefulEntity m_State;
        bool m_HaveUpgradeConsent = false;
    };
}

Q_DECLARE_METATYPE(Connectivity::UpdateService*)

#endif // UPDATESERVICE_H
