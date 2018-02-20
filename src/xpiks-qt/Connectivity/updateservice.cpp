/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "updateservice.h"
#include "updatescheckerworker.h"
#include <QFile>
#include <QString>
#include "../Common/defines.h"
#include "../Models/settingsmodel.h"
#include "../Common/version.h"
#include "../Models/switchermodel.h"

namespace Connectivity {
    UpdateService::UpdateService(Models::SettingsModel *settingsModel,
                                 Models::SwitcherModel *switcherModel,
                                 Maintenance::MaintenanceService *maintenanceService):
        m_UpdatesCheckerWorker(nullptr),
        m_SettingsModel(settingsModel),
        m_SwitcherModel(switcherModel),
        m_MaintenanceService(maintenanceService),
        m_State("updater")
    {
        Q_ASSERT(settingsModel != nullptr);
        Q_ASSERT(switcherModel != nullptr);
        Q_ASSERT(maintenanceService != nullptr);
    }

    void UpdateService::startChecking() {
        const bool startWorker = m_SettingsModel->getCheckForUpdates() &&
                m_SwitcherModel->getUpdateEnabled();

        if (startWorker) {
            updateSettings();
            auto pathToUpdate = getPathToUpdate();
            doStartChecking(pathToUpdate);
        } else {
            LOG_INFO << "Update checking disabled";
        }
    }

    void UpdateService::stopChecking() {
        LOG_DEBUG << "#";
        emit cancelRequested();
    }

    void UpdateService::doStartChecking(const QString &pathToUpdate) {
        LOG_DEBUG << "path to update:" << pathToUpdate;

        m_UpdatesCheckerWorker = new UpdatesCheckerWorker(m_SettingsModel, m_MaintenanceService, pathToUpdate);
        QThread *thread = new QThread();
        m_UpdatesCheckerWorker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, m_UpdatesCheckerWorker, &UpdatesCheckerWorker::process);
        QObject::connect(m_UpdatesCheckerWorker, &UpdatesCheckerWorker::stopped, thread, &QThread::quit);

        QObject::connect(m_UpdatesCheckerWorker, &UpdatesCheckerWorker::stopped, m_UpdatesCheckerWorker, &UpdatesCheckerWorker::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        QObject::connect(this, &UpdateService::cancelRequested,
                         m_UpdatesCheckerWorker, &UpdatesCheckerWorker::cancelRequested);

        QObject::connect(m_UpdatesCheckerWorker, &UpdatesCheckerWorker::updateAvailable,
                         this, &UpdateService::updateAvailable);
        QObject::connect(m_UpdatesCheckerWorker, &UpdatesCheckerWorker::updateDownloaded,
                         this, &UpdateService::updateDownloadedHandler);

        QObject::connect(m_UpdatesCheckerWorker, &UpdatesCheckerWorker::stopped,
                         this, &UpdateService::workerFinished);

        thread->start(QThread::LowPriority);
    }

    void UpdateService::updateSettings() {
        LOG_DEBUG << "#";

        const int availableValue = getAvailableUpdateVersion();
        LOG_DEBUG << "Available:" << availableValue << "current:" << XPIKS_VERSION_INT;

        if ((0 < availableValue) && (availableValue <= XPIKS_VERSION_INT)) {
            LOG_DEBUG << "Flushing available update settings values";
            setPathToUpdate("");
            setAvailableUpdateVersion(0);
        }
    }

    void UpdateService::workerFinished() {
        LOG_DEBUG << "#";
    }

    void UpdateService::updateDownloadedHandler(const QString &updatePath, int version) {
        LOG_DEBUG << "#";

        setAvailableUpdateVersion(version);
        setPathToUpdate(updatePath);

        emit updateDownloaded(updatePath);
    }
}
