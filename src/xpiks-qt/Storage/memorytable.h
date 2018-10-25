/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MEMORYTABLE_H
#define MEMORYTABLE_H

#include <functional>

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QVector>

#include "Storage/idatabase.h"

template <class T1, class T2> struct QPair;

namespace Storage {
    class MemoryTable: public IDbTable
    {
    public:
        MemoryTable(const QString &name);

        // IDbTable interface
    public:
        virtual const QString &getTableName() const override { return m_Name; }
        virtual bool tryGetValue(const QByteArray &key, QByteArray &value) override;
        virtual bool trySetValue(const QByteArray &key, const QByteArray &value) override;
        virtual bool tryAddValue(const QByteArray &key, const QByteArray &value) override;
        virtual bool trySetMany(const QVector<QPair<QByteArray, QByteArray> > &keyValueList, QVector<int> &) override;
        virtual int tryAddMany(const QVector<QPair<QByteArray, QByteArray> > &keyValueList) override;
        virtual bool tryDeleteRecord(const QByteArray &key) override;
        virtual bool tryDeleteMany(const QVector<QByteArray> &keysList) override;
        virtual void foreachRow(const std::function<bool (const QByteArray &, QByteArray &)> &action) override;

    private:
        QHash<QByteArray, QByteArray> m_Dictionary;
        QString m_Name;
    };
}

#endif // MEMORYTABLE_H
