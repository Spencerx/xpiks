/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QThread>
#include <Common/defines.h>
#include <Common/logging.h>
#include "maintenanceservice.h"
#include "maintenanceworker.h"
#include "logscleanupjobitem.h"
#include "updatescleanupjobitem.h"
#include "launchexiftooljobitem.h"
#include "initializedictionariesjobitem.h"
#include "movesettingsjobitem.h"
#include "savesessionjobitem.h"
#include "moveimagecachejobitem.h"
#include "xpkscleanupjob.h"
#include "updatebundlecleanupjobitem.h"
#include <Helpers/asynccoordinator.h>
#include <Artworks/artworkssnapshot.h>

namespace Maintenance {
    MaintenanceService::MaintenanceService(Common::ISystemEnvironment &environment):
        m_Environment(environment),
        m_MaintenanceThread(nullptr),
        m_MaintenanceWorker(nullptr),
        m_LastSessionBatchId(INVALID_BATCH_ID)
    {
    }

    void MaintenanceService::startService() {
        LOG_DEBUG << "#";

        if (m_MaintenanceWorker != NULL) {
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
        if (m_MaintenanceWorker != NULL) {
            m_MaintenanceWorker->stopWorking();
        } else {
            LOG_WARNING << "MaintenanceWorker is NULL";
        }
    }

#ifdef INTEGRATION_TESTS
    bool MaintenanceService::hasPendingJobs() {
        if (m_MaintenanceWorker != nullptr) {
            return m_MaintenanceWorker->hasPendingJobs();
        } else {
            return false;
        }
    }

    void MaintenanceService::cleanup() {
        m_MaintenanceWorker->cancelBatch(m_LastSessionBatchId);
    }
#endif

    void MaintenanceService::cleanupUpdatesArtifacts() {
#ifdef Q_OS_WIN
        LOG_DEBUG << "#";
        std::shared_ptr<IMaintenanceItem> jobItem(new UpdatesCleanupJobItem());
        m_MaintenanceWorker->submitItem(jobItem);
#endif
    }

    void MaintenanceService::cleanupDownloadedUpdates(const QString &downloadsPath) {
        LOG_DEBUG << "#";
        std::shared_ptr<IMaintenanceItem> jobItem(new UpdateBundleCleanupJobItem(downloadsPath));
        m_MaintenanceWorker->submitItem(jobItem);
    }

    void MaintenanceService::launchExiftool(const QString &settingsExiftoolPath) {
        LOG_DEBUG << "#";
        Q_ASSERT(m_MaintenanceThread != nullptr);
        auto jobItem = std::make_shared<LaunchExiftoolJobItem>(settingsExiftoolPath);
        QObject::connect(jobItem.get(), &LaunchExiftoolJobItem::exiftoolDetected,
                         this, &MaintenanceService::exiftoolDetected);
        jobItem->moveToThread(m_MaintenanceThread);
        m_MaintenanceWorker->submitItem(jobItem);
    }

    void MaintenanceService::initializeDictionaries(Translation::TranslationManager *translationManager, Helpers::AsyncCoordinator *initCoordinator) {
        LOG_DEBUG << "#";
        Helpers::AsyncCoordinatorLocker locker(initCoordinator);
        Q_UNUSED(locker);
        std::shared_ptr<IMaintenanceItem> jobItem(new InitializeDictionariesJobItem(translationManager, initCoordinator));
        m_MaintenanceWorker->submitFirst(jobItem);
    }

    void MaintenanceService::cleanupLogs() {
#ifdef WITH_LOGS
        LOG_DEBUG << "#";
        std::shared_ptr<IMaintenanceItem> jobItem(new LogsCleanupJobItem(m_Environment));
        m_MaintenanceWorker->submitItem(jobItem);
#endif
    }

    void MaintenanceService::moveSettings(Models::SettingsModel *settingsModel) {
        std::shared_ptr<IMaintenanceItem> jobItem(new MoveSettingsJobItem(settingsModel));
        m_MaintenanceWorker->submitFirst(jobItem);
    }

    void MaintenanceService::upgradeImagesCache(QMLExtensions::ImageCachingService *imageCachingService) {
        std::shared_ptr<IMaintenanceItem> jobItem(new MoveImageCacheJobItem(imageCachingService));
        m_MaintenanceWorker->submitItem(jobItem);
    }

    void MaintenanceService::saveSession(std::unique_ptr<MetadataIO::SessionSnapshot> &sessionSnapshot, Models::SessionManager *sessionManager) {
        LOG_DEBUG << "#";

        std::shared_ptr<IMaintenanceItem> jobItem(new SaveSessionJobItem(sessionSnapshot, sessionManager));
        m_LastSessionBatchId = m_MaintenanceWorker->submitItem(jobItem);
    }

    void MaintenanceService::cleanupOldXpksBackups(const QString &directory) {
        LOG_DEBUG << directory;

        std::shared_ptr<IMaintenanceItem> jobItem(new XpksCleanupJob(directory));
        m_MaintenanceWorker->submitItem(jobItem);
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
        m_MaintenanceWorker = NULL;
    }
}
