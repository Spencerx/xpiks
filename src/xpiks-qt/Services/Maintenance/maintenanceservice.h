/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MAINTENANCESERVICE_H
#define MAINTENANCESERVICE_H

#include <atomic>
#include <memory>

#include <QObject>
#include <QString>

#include "Common/itemprocessingworker.h"
#include "Common/messages.h"
#include "Common/statefulentity.h"
#include "Common/types.h"
#include "Connectivity/analyticsuserevent.h"
#include "Services/Maintenance/maintenanceworker.h"

class QThread;

namespace Artworks {
    class SessionSnapshot;
}

namespace Common {
    class ISystemEnvironment;
}

namespace Translation {
    class TranslationManager;
}

namespace Models {
    class SessionManager;
}

namespace Helpers {
    class AsyncCoordinator;
}

namespace Maintenance {
    class MaintenanceService: public QObject, public Common::MessagesSource<Common::NamedType<Connectivity::EventType>>
    {
        Q_OBJECT
    public:
        MaintenanceService(Common::ISystemEnvironment &environment);

    public:
        void startService();
        void stopService();

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
    public:
        bool hasPendingJobs();
        bool isWorkerDestroyed() const { return m_MaintenanceWorker == nullptr; }
        void cleanup();
#endif

    public:
        void cleanupCrashDumps();
        void cleanupUpdatesArtifacts();
        void cleanupDownloadedUpdates(const QString &downloadsPath);
        void launchExiftool(const QString &exiftoolPath);
        void initializeDictionaries(Translation::TranslationManager &translationManager,
                                    Helpers::AsyncCoordinator &initCoordinator);
        void cleanupLogs();
        void saveSession(std::unique_ptr<Artworks::SessionSnapshot> &sessionSnapshot,
                         Models::SessionManager &sessionManager);
        void cleanupOldXpksBackups(const QString &directory);

    private:
        bool isRunning();

    signals:
        void exiftoolDetected(const QString &path);

    public slots:
        void onExiftoolPathChanged(const QString &path);

    private slots:
        void workerFinished();
        void workerDestroyed(QObject *object);
        void lastCrashFound(QString crashFilePath);

    private:
        QThread *m_MaintenanceThread;
        Common::StatefulEntity m_State;
        Common::ISystemEnvironment &m_Environment;
        MaintenanceWorker *m_MaintenanceWorker;
        MaintenanceWorker::batch_id_t m_LastSessionBatchId;
        std::atomic_bool m_IsStopped;
    };
}

#endif // MAINTENANCESERVICE_H
