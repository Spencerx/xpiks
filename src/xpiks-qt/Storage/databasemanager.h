/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QByteArray>
#include <QReadWriteLock>
#include <QDataStream>
#include <QAtomicInt>
#include <QMutex>
#include <QHash>
#include <QString>
#include <QVector>
#include <QPair>
#include <vector>
#include <memory>
#include <functional>
#include "../Helpers/asynccoordinator.h"
#include "../Common/defines.h"
#include "../Common/isystemenvironment.h"
#include "idatabasemanager.h"
#include "database.h"

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
        std::shared_ptr<IDatabase> doOpenDatabase(const QString &root, const QString &dbName);

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
        QAtomicInt m_LastDatabaseID;
        std::vector<std::shared_ptr<IDatabase> > m_DatabaseArray;
        volatile bool m_Initialized;
    };
}

#endif // DATABASEMANAGER_H
