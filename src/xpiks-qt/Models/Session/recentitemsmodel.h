/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef RECENTITEMSMODEL_H
#define RECENTITEMSMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QModelIndex>
#include <QObject>
#include <QQueue>
#include <QSet>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <Qt>
#include <QtGlobal>

#include "Common/lrucache.h"
#include "Helpers/hashhelpers.h"  // IWYU pragma: keep

class QByteArray;
class QModelIndex;

namespace Models {
    class RecentItemsModel: public QAbstractListModel
    {
        Q_OBJECT
        Q_PROPERTY(int count READ getRecentItemsCount NOTIFY recentItemsCountChanged)

    public:
        RecentItemsModel(size_t count);

    public:
        int getRecentItemsCount() const { return m_RecentItems.size(); }
        size_t getMaxRecentItems() const { return m_MaxRecentItems; }

    public:
        QString serializeItems();

    public:
        virtual void initialize() = 0;

    public:
        void deserializeItems(const QString &serialized);
        void pushItem(const QString &item);

    private:
        void rebuild();

#ifdef CORE_TESTS
    public:
        QString getLatestUsedItem() const { return m_LRUcache.data().begin()->first; }
        bool contains(const QString &s) const { return m_RecentItems.contains(s); }
#endif

    protected:
        virtual void sync() = 0;

    public:
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    protected:
        virtual QHash<int, QByteArray> roleNames() const override { return QAbstractListModel::roleNames(); }
        const QStringList &getRecentItems() const { return m_RecentItems; }

    signals:
        void recentItemsCountChanged();

    private:
        Common::LRUCache<QString, int> m_LRUcache;
        QStringList m_RecentItems;
        size_t m_MaxRecentItems;
    };
}

#endif // RECENTITEMSMODEL_H
