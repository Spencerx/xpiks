/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef WRITEAHEADLOG_H
#define WRITEAHEADLOG_H

#include <QDataStream>
#include <QHash>
#include <QReadWriteLock>

#include "Common/logging.h"
#include "Storage/idatabase.h"

namespace Storage {
    template<class TKey, class TValue>
    class WriteAheadLog {
    public:
        bool tryGet(const TKey &key, TValue &value) {
            QReadLocker locker(&m_LockWAL);
            Q_UNUSED(locker);

            bool contains = false;
            if (m_WriteAheadLog.contains(key)) {
                contains = true;
                value = m_WriteAheadLog[key];
            }

            return contains;
        }

        void set(const TKey &key, const TValue &value) {
            QWriteLocker walLocker(&m_LockWAL);
            Q_UNUSED(walLocker);

            m_WriteAheadLog.insert(key, value);
        }

        void setMany(const QHash<TKey, TValue> &existing) {
            if (existing.isEmpty()) { return; }

            QWriteLocker locker(&m_LockWAL);
            Q_UNUSED(locker);

            auto it = existing.begin();
            auto itEnd = existing.end();
            while (it != itEnd) {
                m_WriteAheadLog.insert(it.key(), it.value());
                it++;
            }
        }

        void flush(std::shared_ptr<IDbTable> &dbTable) {
            LOG_DEBUG << "#";
            if (m_WriteAheadLog.empty()) { return; }

            QVector<QPair<QByteArray, QByteArray> > keyValuesList;
            {
                QWriteLocker walLocker(&m_LockWAL);
                Q_UNUSED(walLocker);

                auto it = m_WriteAheadLog.begin();
                auto itEnd = m_WriteAheadLog.end();
                for (; it != itEnd; ++it) {
                    const TKey &key = it.key();
                    const TValue &value = it.value();

                    QByteArray rawKey = keyToByteArray(key);
                    QByteArray rawValue;

                    QDataStream ds(&rawValue, QIODevice::WriteOnly);
                    ds << value;
                    Q_ASSERT(ds.status() != QDataStream::WriteFailed);

                    if (ds.status() != QDataStream::WriteFailed) {
                        keyValuesList.push_back(QPair<QByteArray, QByteArray>(rawKey, rawValue));
                    }
                }

                m_WriteAheadLog.clear();
            }

            if (keyValuesList.isEmpty()) { return; }

            QVector<int> failedIndices;
            bool success = doFlush(dbTable, keyValuesList, failedIndices);
            if (success) {
                LOG_INFO << "WAL has been flushed successfully";
                Q_ASSERT(failedIndices.isEmpty());
            } else {
                LOG_WARNING << "Failed to flush WAL successfully. Restoring failed items...";
                QWriteLocker locker(&m_LockWAL);
                Q_UNUSED(locker);
                restoreFailedItems(keyValuesList, failedIndices);
            }
        }

        int size() {
            QReadLocker locker(&m_LockWAL);
            Q_UNUSED(locker);
            return m_WriteAheadLog.size();
        }

    private:
        void restoreFailedItems(QVector<QPair<QByteArray, QByteArray> > &keyValuesList, const QVector<int> &failedIndices) {
            for (auto &index: failedIndices) {
                auto &keyValuePair = keyValuesList[index];

                TKey key = keyFromByteArray(keyValuePair.first);

                TValue value;
                QByteArray &rawData = keyValuePair.second;
                QDataStream ds(&rawData, QIODevice::ReadOnly);
                ds >> value;

                m_WriteAheadLog.insert(key, value);
            }
        }

    protected:
        virtual QByteArray keyToByteArray(const TKey &key) const = 0;
        virtual TKey keyFromByteArray(const QByteArray &rawKey) const = 0;
        virtual bool doFlush(std::shared_ptr<IDbTable> &dbTable, const QVector<QPair<QByteArray, QByteArray> > &keyValuesList, QVector<int> &failedIndices) = 0;

    private:
        QReadWriteLock m_LockWAL;
        QHash<TKey, TValue> m_WriteAheadLog;
    };
}

#endif // WRITEAHEADLOG_H
