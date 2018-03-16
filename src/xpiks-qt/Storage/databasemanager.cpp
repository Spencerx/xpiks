/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "databasemanager.h"
#include <QDir>
#include "../Helpers/constants.h"

namespace Storage {
    DatabaseManager::DatabaseManager(Common::ISystemEnvironment &environment):
        QObject(),
        m_Environment(environment),
        m_LastDatabaseID(0),
        m_Initialized(false)
    {
        QObject::connect(&m_FinalizeCoordinator, &Helpers::AsyncCoordinator::statusReported,
                         this, &DatabaseManager::onReadyToFinalize);
    }

    bool DatabaseManager::initialize() {
        LOG_DEBUG << "#";
        m_Environment.ensureDirExists(Constants::DB_DIR);
        QString dbDirPath = m_Environment.path({Constants::DB_DIR});
        m_DBDirPath = dbDirPath;
        m_Initialized = true;
        return true;
    }

#ifdef DEBUG_UTILITY
    bool DatabaseManager::initialize(const QString &dbDirPath) {
        LOG_DEBUG << "#";
        m_DBDirPath = dbDirPath;
        m_Initialized = true;
        return true;
    }
#endif

    void DatabaseManager::finalize() {
        LOG_DEBUG << "#";
        Q_ASSERT(m_Initialized);
        closeAll();
        m_Initialized = false;
    }

    std::shared_ptr<IDatabase> DatabaseManager::openDatabase(const QString &dbName) {
        return doOpenDatabase(m_DBDirPath, dbName);
    }

    std::shared_ptr<IDatabase> DatabaseManager::openDatabase(Common::ISystemEnvironment &environment, const QString &dbName) {
        return doOpenDatabase(environment.path({Constants::DB_DIR}), dbName);
    }

    std::shared_ptr<IDatabase> DatabaseManager::doOpenDatabase(const QString &root, const QString &dbName) {
        Q_ASSERT(m_Initialized);
        LOG_DEBUG << dbName;

        const int id = getNextID();
        std::shared_ptr<IDatabase> db(new Database(id, &m_FinalizeCoordinator));

        QDir databasesDir(root);
        Q_ASSERT(databasesDir.exists());
        QString fullDbPath = databasesDir.filePath(dbName);
        QByteArray utf8Path = fullDbPath.toUtf8();
        const char *dbPath = utf8Path.data();

        bool openSucceded = db->open(dbPath);
        if (!openSucceded) {
            LOG_WARNING << "Failed to open" << dbName;
            db.reset();
        } else {
            LOG_INFO << "Opened" << dbName << "database";

            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);
            m_DatabaseArray.push_back(db);
        }

        return db;
    }

    void DatabaseManager::prepareToFinalize() {
        LOG_DEBUG << "#";
        m_FinalizeCoordinator.allBegun();
    }

    void DatabaseManager::onReadyToFinalize(int status) {
        LOG_INFO << status;
        finalize();
    }

    void DatabaseManager::closeAll() {
        LOG_DEBUG << "#";

        for (auto &db: m_DatabaseArray) {
            db->finalize();
            db->close();
        }

        LOG_INFO << "Databases closed";
    }

    int DatabaseManager::getNextID() {
        int id = m_LastDatabaseID.fetchAndAddOrdered(1);
        return id;
    }
}
