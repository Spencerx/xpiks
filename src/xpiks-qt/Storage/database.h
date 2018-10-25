/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include <memory>
#include <functional>

#include <QStringList>
#include <QByteArray>
#include <QString>
#include <QVector>

#include "Storage/idatabase.h"

template <class T1, class T2> struct QPair;

struct sqlite3;
struct sqlite3_stmt;

namespace Helpers {
    class AsyncCoordinator;
}

namespace Storage {
    // super simple wrapper over sqlite
    // to make it look like a key-value storage
    class Database: public IDatabase {
    public:
        Database(int id, Helpers::AsyncCoordinator &finalizeCoordinator);
        virtual ~Database();

    private:
        class Transaction {
        public:
            Transaction(sqlite3 *database);
            virtual ~Transaction();

        private:
            sqlite3 *m_Database;
            bool m_Started;
        };

    public:
        class Table: public IDbTable {
        public:
            Table(sqlite3 *database, const QString &tableName);

        public:
            bool initialize();
            void finalize();

        public:
            virtual const QString &getTableName() const override { return m_TableName; }
            virtual bool tryGetValue(const QByteArray &key, QByteArray &value) override;
            virtual bool trySetValue(const QByteArray &key, const QByteArray &value) override;
            virtual bool tryAddValue(const QByteArray &key, const QByteArray &value) override;
            virtual bool trySetMany(const QVector<QPair<QByteArray, QByteArray> > &keyValueList, QVector<int> &failedIndices) override;
            virtual int tryAddMany(const QVector<QPair<QByteArray, QByteArray> > &keyValueList) override;
            virtual bool tryDeleteRecord(const QByteArray &key) override;
            virtual bool tryDeleteMany(const QVector<QByteArray> &keysList) override;
            virtual void foreachRow(const std::function<bool (const QByteArray &, QByteArray &)> &action) override;

        private:
            QString m_TableName;
            sqlite3 *m_Database;
            sqlite3_stmt *m_GetStatement;
            sqlite3_stmt *m_SetStatement;
            sqlite3_stmt *m_AddStatement;
            sqlite3_stmt *m_DelStatement;
            sqlite3_stmt *m_AllStatement;
        };

    public:
        virtual bool open(const char *fullDbPath) override;
        virtual void close() override;
        virtual void sync() override;
        virtual std::shared_ptr<IDbTable> getTable(const QString &name) override;
        virtual bool deleteTable(std::shared_ptr<IDbTable> &table) override;
        virtual QStringList retrieveTableNames() override;

    public:
        bool initialize();
        void finalize();

    private:
        void doClose();
        void executeStatement(const char *stmt, bool &anyError);

    private:
        int m_ID;
        Helpers::AsyncCoordinator &m_FinalizeCoordinator;
        sqlite3 *m_Database = nullptr;
        sqlite3_stmt *m_GetTablesStatement = nullptr;
        std::vector<std::shared_ptr<Database::Table> > m_Tables;
        volatile bool m_IsOpened = false;
    };
}

#endif // DATABASE_H
