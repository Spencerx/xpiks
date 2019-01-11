/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <atomic>
#include <memory>
#include <vector>

#include <QMutex>
#include <QObject>
#include <QString>

#include "Helpers/asynccoordinator.h"
#include "Storage/idatabasemanager.h"

namespace Common {
    class ISystemEnvironment;
}

namespace Storage {
    class Database;
    class IDatabase;
}

namespace Storage {
    class DatabaseManager: public QObject, public IDatabaseManager {
        Q_OBJECT
    public:
        DatabaseManager(Common::ISystemEnvironment &environment);

    public:
        bool initialize();

#ifdef DEBUG_UTILITY
    public:
        bool initialize(const QString &dbDirPath);
#endif

    private:
        void finalize();
        int closeEnvironment();

    public:
        virtual std::shared_ptr<IDatabase> openDatabase(const QString &dbName) override;
        std::shared_ptr<IDatabase> openDatabase(Common::ISystemEnvironment &environment, const QString &dbName);

    private:
        std::shared_ptr<Database> openDbFromFile(const QString &root, const QString &dbName);
        std::shared_ptr<Database> openTempDb();

    public:
        void prepareToFinalize();

    private slots:
        void onReadyToFinalize(int status);

    private:
        void closeAll();
        int getNextID();

    private:
        Common::ISystemEnvironment &m_Environment;
        QMutex m_Mutex;
        Helpers::AsyncCoordinator m_FinalizeCoordinator;
        QString m_DBDirPath;
        std::atomic_int m_LastDatabaseID;
        std::vector<std::shared_ptr<Database>> m_DatabaseArray;
        std::atomic_bool m_Initialized;
    };
}

#endif // DATABASEMANAGER_H
