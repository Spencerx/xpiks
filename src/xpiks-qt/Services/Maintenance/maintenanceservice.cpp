/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "maintenanceservice.h"

#include <QThread>
#include <QtDebug>
#include <QtGlobal>

#include "Artworks/artworkssnapshot.h"  // IWYU pragma: keep
#include "Common/defines.h"
#include "Common/logging.h"
#include "Helpers/asynccoordinator.h"
#include "Services/Maintenance/initializedictionariesjobitem.h"
#include "Services/Maintenance/launchexiftooljobitem.h"
#include "Services/Maintenance/logscleanupjobitem.h"
#include "Services/Maintenance/maintenanceworker.h"
#include "Services/Maintenance/savesessionjobitem.h"
#include "Services/Maintenance/updatebundlecleanupjobitem.h"
#include "Services/Maintenance/xpkscleanupjob.h"

#ifdef Q_OS_WIN
#include "Services/Maintenance/updatescleanupjobitem.h"
#endif

namespace Maintenance {
    MaintenanceService::MaintenanceService(Common::ISystemEnvironment &environment):
        m_MaintenanceThread(nullptr),
        m_Environment(environment),
        m_MaintenanceWorker(nullptr),
        m_LastSessionBatchId(INVALID_BATCH_ID),
        m_IsStopped(false)
    {
    }

    void MaintenanceService::startService() {
        LOG_DEBUG << "#";

        if (m_MaintenanceWorker != nullptr) {
            LOG_WARNING << "Attempt to start running worker";
            return;
        }

        m_MaintenanceWorker = new MaintenanceWorker();

        QThread *thread = new QThread();
        m_MaintenanceWorker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, m_MaintenanceWorker, &MaintenanceWorker::process);
        QObject::connect(m_MaintenanceWorker, &MaintenanceWorker::stopped, thread, &QThread::quit);

        QObject::connect(m_MaintenanceWorker, &MaintenanceWorker::stopped, m_MaintenanceWorker, &MaintenanceWorker::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        QObject::connect(m_MaintenanceWorker, &MaintenanceWorker::stopped,
                         this, &MaintenanceService::workerFinished);
        QObject::connect(m_MaintenanceWorker, &MaintenanceWorker::destroyed,
                         this, &MaintenanceService::workerDestroyed);

        LOG_DEBUG << "starting low priority thread...";
        thread->start(QThread::LowPriority);

        m_MaintenanceThread = thread;
    }

    void MaintenanceService::stopService() {
        LOG_DEBUG << "#";
        if (isRunning()) {
            m_MaintenanceWorker->stopWorking();
        } else {
            LOG_WARNING << "MaintenanceWorker is nullptr";
        }
        m_IsStopped = true;
    }

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
    bool MaintenanceService::hasPendingJobs() {
        if (m_MaintenanceWorker != nullptr) {
            return m_MaintenanceWorker->hasPendingJobs();
        } else {
            return false;
        }
    }

    void MaintenanceService::cleanup() {
        if (!isRunning()) { return; }
        m_MaintenanceWorker->cancelBatch(m_LastSessionBatchId);
    }
#endif

    void MaintenanceService::cleanupUpdatesArtifacts() {
#ifdef Q_OS_WIN
        LOG_DEBUG << "#";
        if (!isRunning()) { return; }
        auto jobItem = std::make_shared<UpdatesCleanupJobItem>();
        m_MaintenanceWorker->submitItem(jobItem);
#endif
    }

    void MaintenanceService::cleanupDownloadedUpdates(const QString &downloadsPath) {
        LOG_DEBUG << "#";
        if (!isRunning()) { return; }
        auto jobItem = std::make_shared<UpdateBundleCleanupJobItem>(downloadsPath);
        m_MaintenanceWorker->submitItem(jobItem);
    }

    void MaintenanceService::launchExiftool(const QString &settingsExiftoolPath) {
        LOG_INFO << settingsExiftoolPath;
        if (!isRunning()) { return; }
        Q_ASSERT(m_MaintenanceThread != nullptr);
        auto jobItem = std::make_shared<LaunchExiftoolJobItem>(settingsExiftoolPath);
        QObject::connect(jobItem.get(), &LaunchExiftoolJobItem::exiftoolDetected,
                         this, &MaintenanceService::exiftoolDetected);
        jobItem->moveToThread(m_MaintenanceThread);
        m_MaintenanceWorker->submitItem(jobItem);
    }

    void MaintenanceService::initializeDictionaries(Translation::TranslationManager &translationManager,
                                                    Helpers::AsyncCoordinator &initCoordinator) {
        LOG_DEBUG << "#";
        if (!isRunning()) { return; }
        Helpers::AsyncCoordinatorLocker locker(initCoordinator);
        Q_UNUSED(locker);
        auto jobItem = std::make_shared<InitializeDictionariesJobItem>(translationManager, initCoordinator);
        m_MaintenanceWorker->submitFirst(jobItem);
    }

    void MaintenanceService::cleanupLogs() {
#ifdef WITH_LOGS
        LOG_DEBUG << "#";
        if (!isRunning()) { return; }
        auto jobItem = std::make_shared<LogsCleanupJobItem>(m_Environment);
        m_MaintenanceWorker->submitItem(jobItem);
#endif
    }

    void MaintenanceService::saveSession(std::unique_ptr<Artworks::SessionSnapshot> &sessionSnapshot,
                                         Models::SessionManager &sessionManager) {
        LOG_DEBUG << "#";
        if (!isRunning()) { return; }
        auto jobItem = std::make_shared<SaveSessionJobItem>(sessionSnapshot, sessionManager);
        m_LastSessionBatchId = m_MaintenanceWorker->submitItem(jobItem);
    }

    void MaintenanceService::cleanupOldXpksBackups(const QString &directory) {
        LOG_DEBUG << directory;
        if (m_IsStopped) { return; }
        auto jobItem = std::make_shared<XpksCleanupJob>(directory);
        m_MaintenanceWorker->submitItem(jobItem);
    }

    bool MaintenanceService::isRunning() {
        return m_MaintenanceWorker != nullptr && !m_IsStopped;
    }

    void MaintenanceService::onExiftoolPathChanged(const QString &path) {
        LOG_DEBUG << "#";
        launchExiftool(path);
    }

    void MaintenanceService::workerFinished() {
        LOG_DEBUG << "#";
    }

    void MaintenanceService::workerDestroyed(QObject *object) {
        Q_UNUSED(object);
        LOG_DEBUG << "#";
        m_MaintenanceWorker = nullptr;
    }
}
