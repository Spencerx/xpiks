/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "recentitemsmodel.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QList>
#include <QUrl>
#include <QtDebug>

#include "Common/logging.h"

namespace Models {
    RecentItemsModel::RecentItemsModel(size_t count):
        QAbstractListModel(),
        m_LRUcache(count, 0 /*elasticity*/),
        m_MaxRecentItems(count)
    {
    }

    QString RecentItemsModel::serializeItems() {
        // historical reasons to have QQueue here
        // now only thing left is to be backward compatible
        QQueue<QString> items;
        for (auto &kv: m_LRUcache.data()) {
            items.enqueue(kv.first);
        }
        Q_ASSERT(m_LRUcache.size() <= m_MaxRecentItems);
        QByteArray raw;
        QDataStream ds(&raw, QIODevice::WriteOnly);
        ds << items;
        return QString::fromLatin1(raw.toBase64());
    }

    void RecentItemsModel::deserializeItems(const QString &serialized) {
        LOG_DEBUG << "#";

        QByteArray originalData;
        originalData.append(serialized.toLatin1());
        QByteArray serializedBA = QByteArray::fromBase64(originalData);

        // historical reasons to have QQueue there
        // now only thing left is to be backward compatible
        QQueue<QString> items;
        QDataStream ds(&serializedBA, QIODevice::ReadOnly);
        ds >> items;

        Q_ASSERT(m_LRUcache.size() == 0);
        for (auto it = items.rbegin(); it != items.rend(); it++) {
            m_LRUcache.put(*it, 0);
        }
        rebuild();
        emit recentItemsCountChanged();
    }

    void RecentItemsModel::pushItem(const QString &item) {
        LOG_INFO << item;
        m_LRUcache.put(item, 0);
        rebuild();
        sync();
        emit recentItemsCountChanged();
    }

    void RecentItemsModel::rebuild() {
        QStringList next;
        for (const auto &kv: m_LRUcache.data()) {
            next.append(kv.first);
        }
        beginResetModel();
        {
            m_RecentItems.swap(next);
        }
        endResetModel();
        Q_ASSERT(m_RecentItems.size() <= static_cast<int>(m_MaxRecentItems));
    }

    int RecentItemsModel::rowCount(const QModelIndex &parent) const {
         Q_UNUSED(parent);
         return m_RecentItems.size();
    }

    QVariant RecentItemsModel::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if (row < 0 || row >= m_RecentItems.size()) return QVariant();
        if (role == Qt::DisplayRole) { return m_RecentItems.at(index.row()); }
        return QVariant();
    }
}
