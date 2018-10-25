/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "memorytable.h"

#include <QPair>
#include <QtDebug>

#include "Common/logging.h"

namespace Storage {
    MemoryTable::MemoryTable(const QString &name):
        m_Name(name)
    {
    }

    bool MemoryTable::tryGetValue(const QByteArray &key, QByteArray &value) {
        LOG_INFO << key;
        bool found = false;
        auto it = m_Dictionary.find(key);
        if (it != m_Dictionary.end()) {
            value = it.value();
            found = true;
        }

        return found;
    }

    bool MemoryTable::trySetValue(const QByteArray &key, const QByteArray &value) {
        LOG_INFO << key << "<-" << value;
        m_Dictionary.insert(key, value);
        return true;
    }

    bool MemoryTable::tryAddValue(const QByteArray &key, const QByteArray &value) {
        LOG_INFO << key << "," << value;
        m_Dictionary.insert(key, value);
        return true;
    }

    bool MemoryTable::trySetMany(const QVector<QPair<QByteArray, QByteArray> > &keyValueList, QVector<int> &) {
        LOG_DEBUG << "#";
        bool anyFailed = false;
        for (auto &item: keyValueList) {
            bool success = trySetValue(item.first, item.second);
            if (!success) { anyFailed = true; }
        }
        return !anyFailed;
    }

    int MemoryTable::tryAddMany(const QVector<QPair<QByteArray, QByteArray> > &keyValueList) {
        LOG_DEBUG << "#";
        bool anyFailed = false;
        for (auto &item: keyValueList) {
            bool success = tryAddValue(item.first, item.second);
            if (!success) { anyFailed = true; }
        }
        return !anyFailed;
    }

    bool MemoryTable::tryDeleteRecord(const QByteArray &key) {
        LOG_INFO << key;
        int removedCount = m_Dictionary.remove(key);
        return (removedCount > 0);
    }

    bool MemoryTable::tryDeleteMany(const QVector<QByteArray> &keysList) {
        LOG_DEBUG << "#";
        bool anyFailed = false;
        for (auto &key: keysList) {
            bool success = tryDeleteRecord(key);
            if (!success) { anyFailed = true; }
        }
        return !anyFailed;
    }

    void MemoryTable::foreachRow(const std::function<bool (const QByteArray &, QByteArray &)> &action) {
        LOG_DEBUG << "#";
        auto it = m_Dictionary.begin();
        auto itEnd = m_Dictionary.end();
        while (it != itEnd) {
            const bool shouldContinue = action(it.key(), it.value());
            if (!shouldContinue) { break; }
            it++;
        }
    }
}
