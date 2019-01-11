/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "databasemanager.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QMutexLocker>
#include <QtDebug>
#include <QtGlobal>

#include "Common/isystemenvironment.h"
#include "Common/logging.h"
#include "Helpers/asynccoordinator.h"
#include "Helpers/constants.h"
#include "Storage/database.h"

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
        std::shared_ptr<Database> db;

        if (!m_Environment.getIsInMemoryOnly()) {
            db = openDbFromFile(m_DBDirPath, dbName);
        }

        if (!db) {
            LOG_WARNING << "Failed to open DB" << dbName << "from file";
            db = openTempDb();
        }

        auto result = std::dynamic_pointer_cast<IDatabase>(db);
        return result;
    }

    std::shared_ptr<IDatabase> DatabaseManager::openDatabase(Common::ISystemEnvironment &environment, const QString &dbName) {
        std::shared_ptr<Database> db;

        if (!environment.getIsInMemoryOnly()) {
            db = openDbFromFile(environment.path({Constants::DB_DIR}), dbName);
        }

        if (!db) {
            LOG_WARNING << "Failed to open DB" << dbName << "from file";
            db = openTempDb();
        }

        auto result = std::dynamic_pointer_cast<IDatabase>(db);
        return result;
    }

    std::shared_ptr<Database> DatabaseManager::openDbFromFile(const QString &root, const QString &dbName) {
        Q_ASSERT(m_Initialized);

        const int id = getNextID();
        LOG_INFO << "Using #" << id << "for DB" << dbName;

        QDir databasesDir(root);
        Q_ASSERT(databasesDir.exists());
        QString fullDbPath = databasesDir.filePath(dbName);
        QByteArray utf8Path = fullDbPath.toUtf8();
        const char *dbPath = utf8Path.data();
        std::shared_ptr<Database> db;
        bool success = false, triedHard = false;

        do {
            db = std::make_shared<Database>(id, m_FinalizeCoordinator);
            if (db->open(dbPath)) {
                success = db->initialize();
            }

            if (!success) { db->close(); }
            if (triedHard || success) { break; }

            bool removed = QFile(fullDbPath).remove();
            LOG_DEBUG << "File" << fullDbPath << "removed:" << removed;
            triedHard = true;
        } while (!success);

        if (success) {
            LOG_INFO << "Opened and initialized" << dbName << "database";

            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);
            m_DatabaseArray.push_back(db);
        } else {
            db.reset();
        }

        return db;
    }

    std::shared_ptr<Database> DatabaseManager::openTempDb() {
        Q_ASSERT(m_Initialized);

        const int id = getNextID();
        LOG_INFO << "Using #" << id << "for temp DB";

        auto db = std::make_shared<Database>(id, m_FinalizeCoordinator);
        bool success = false;

        // empty string means temp db
        if (db->open("")) {
            success = db->initialize();
        }

        if (!success) {
            db->close();
            db = std::make_shared<Database>(id, m_FinalizeCoordinator);

            // in-memory db
            if (db->open(":memory:")) {
                success = db->initialize();
            }
        }

        if (success) {
            LOG_INFO << "Opened and initialized temporary database";

            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);
            m_DatabaseArray.push_back(db);
        } else {
            db.reset();
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
            db->close();
        }

        LOG_INFO << "Databases closed";
    }

    int DatabaseManager::getNextID() {
        int id = m_LastDatabaseID.fetch_add(1);
        return id;
    }
}
