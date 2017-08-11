/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef RECENTDIRECTORIESMODEL_H
#define RECENTDIRECTORIESMODEL_H

#include <QAbstractListModel>
#include <QQueue>
#include <QString>
#include <QSet>
#include <QHash>
#include <QUrl>

namespace Models {
    class RecentDirectoriesModel : public QAbstractListModel
    {
        Q_OBJECT
    public:
        RecentDirectoriesModel();

    public:
        Q_INVOKABLE QString serializeForSettings();
        void deserializeFromSettings(const QString &serialized);
        void pushDirectory(const QString &directoryPath);
        Q_INVOKABLE QUrl getLatestDirectory() const;
        int getMaxRecentDirectories() const { return m_MaxRecentDirectories; }

#ifdef CORE_TESTS
        QString getLatestUsedDirectory() const { return m_LatestUsedDirectory; }
#endif

    private:
        bool doPushDirectory(const QString &directoryPath);

    public:
        int rowCount(const QModelIndex & parent = QModelIndex()) const override { Q_UNUSED(parent); return m_RecentDirectories.length(); }
        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    protected:
        virtual QHash<int, QByteArray> roleNames() const override { return QAbstractListModel::roleNames(); }

    private:
        QSet<QString> m_DirectoriesSet;
        QQueue<QString> m_RecentDirectories;
        int m_MaxRecentDirectories;
        QString m_LatestUsedDirectory;
    };
}

#endif // RECENTDIRECTORIESMODEL_H
