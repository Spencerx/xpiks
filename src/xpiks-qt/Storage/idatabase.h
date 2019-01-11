/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IDATABASE_H
#define IDATABASE_H

#include <functional>
#include <memory>

#include <QByteArray>
#include <QString>
#include <QStringList>

template <class T1, class T2> struct QPair;
template <typename T> class QVector;

namespace Storage {
    class IDbTable {
    public:
        virtual ~IDbTable() {}
    public:
        virtual const QString &getTableName() const = 0;
        virtual bool tryGetValue(const QByteArray &key, QByteArray &value) = 0;
        virtual bool trySetValue(const QByteArray &key, const QByteArray &value) = 0;
        virtual bool tryAddValue(const QByteArray &key, const QByteArray &value) = 0;
        virtual bool trySetMany(const QVector<QPair<QByteArray, QByteArray> > &keyValueList, QVector<int> &failedIndices) = 0;
        virtual int tryAddMany(const QVector<QPair<QByteArray, QByteArray> > &keyValueList) = 0;
        virtual bool tryDeleteRecord(const QByteArray &key) = 0;
        virtual bool tryDeleteMany(const QVector<QByteArray> &keysList) = 0;
        virtual void foreachRow(const std::function<bool (const QByteArray &, QByteArray &)> &action) = 0;
    };

    class IDatabase {
    public:
        virtual ~IDatabase() {}

    public:
        virtual bool open(const char *fullDbPath) = 0;
        virtual void close() = 0;
        virtual void sync() = 0;
        virtual std::shared_ptr<IDbTable> getTable(const QString &name) = 0;
        virtual bool deleteTable(std::shared_ptr<IDbTable> &table) = 0;
        virtual QStringList retrieveTableNames() = 0;
    };
}

#endif // IDATABASE_H
