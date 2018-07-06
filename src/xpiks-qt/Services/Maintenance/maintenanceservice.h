/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MAINTENANCESERVICE_H
#define MAINTENANCESERVICE_H

#include <QObject>
#include <Common/isystemenvironment.h>
#include "maintenanceworker.h"

namespace Artworks {
    class ArtworkMetadata;
    class SessionSnapshot;
}

namespace Translation {
    class TranslationManager;
}

namespace Models {
    class SettingsModel;
    class SessionManager;
}

namespace Helpers {
    class AsyncCoordinator;
}

namespace QMLExtensions {
    class ImageCachingService;
}

class QThread;

namespace Maintenance {
    class MaintenanceService: public QObject
    {
        Q_OBJECT
    public:
        MaintenanceService(Common::ISystemEnvironment &environment);

    public:
        void startService();
        void stopService();

#ifdef INTEGRATION_TESTS
    public:
        bool hasPendingJobs();
        void cleanup();
#endif

    public:
        void cleanupUpdatesArtifacts();
        void cleanupDownloadedUpdates(const QString &downloadsPath);
        void launchExiftool(const QString &settingsExiftoolPath);
        void initializeDictionaries(Translation::TranslationManager *translationManager,
                                    Helpers::AsyncCoordinator *initCoordinator);
        void cleanupLogs();
        void moveSettings(Models::SettingsModel *settingsModel);
        void upgradeImagesCache(QMLExtensions::ImageCachingService *imageCachingService);
        void saveSession(std::unique_ptr<Artworks::SessionSnapshot> &sessionSnapshot,
                         Models::SessionManager &sessionManager);
        void cleanupOldXpksBackups(const QString &directory);

    signals:
        void exiftoolDetected(const QString &path);

    public slots:
        void onExiftoolPathChanged(const QString &path);

    private slots:
        void workerFinished();
        void workerDestroyed(QObject *object);

    private:
        QThread *m_MaintenanceThread;
        Common::ISystemEnvironment &m_Environment;
        MaintenanceWorker *m_MaintenanceWorker;
        MaintenanceWorker::batch_id_t m_LastSessionBatchId;
    };
}

#endif // MAINTENANCESERVICE_H
