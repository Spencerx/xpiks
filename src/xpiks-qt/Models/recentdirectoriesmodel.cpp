/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "recentdirectoriesmodel.h"
#include <QDataStream>
#include <QUrl>
#include "../Common/defines.h"

namespace Models {
    RecentDirectoriesModel::RecentDirectoriesModel():
        QAbstractListModel(),
        m_MaxRecentDirectories(5)
    {
    }

    QString RecentDirectoriesModel::serializeForSettings() {
        QByteArray raw;
        QDataStream ds(&raw, QIODevice::WriteOnly);
        ds << m_RecentDirectories;
        return QString::fromLatin1(raw.toBase64());
    }

    void RecentDirectoriesModel::deserializeFromSettings(const QString &serialized) {
        LOG_DEBUG << "#";

        QByteArray originalData;
        originalData.append(serialized.toLatin1());
        QByteArray serializedBA = QByteArray::fromBase64(originalData);

        QQueue<QString> items;
        QDataStream ds(&serializedBA, QIODevice::ReadOnly);
        ds >> items;

        QQueue<QString> deserialized;
        QSet<QString> toBeAdded;

        foreach (const QString &item, items) {
            if (!toBeAdded.contains(item)) {
                toBeAdded.insert(item);
                deserialized.push_back(item);
            }
        }

        m_DirectoriesSet = toBeAdded;
        m_RecentDirectories = deserialized;
    }

    void RecentDirectoriesModel::pushDirectory(const QString &directoryPath) {
        if (doPushDirectory(directoryPath)) {
            LOG_DEBUG << "Added new recent directory";
        }

        m_LatestUsedDirectory = directoryPath;
    }

    QUrl RecentDirectoriesModel::getLatestDirectory() const {
        return QUrl::fromLocalFile(m_LatestUsedDirectory);
    }

    bool RecentDirectoriesModel::doPushDirectory(const QString &directoryPath) {
        bool added = false;

        if (!m_DirectoriesSet.contains(directoryPath)) {
            m_DirectoriesSet.insert(directoryPath);

            int length = m_RecentDirectories.length();
            beginInsertRows(QModelIndex(), length, length);
            m_RecentDirectories.push_back(directoryPath);
            endInsertRows();

            if (m_RecentDirectories.length() > m_MaxRecentDirectories) {
                QString directoryToRemove = m_RecentDirectories.first();
                beginRemoveRows(QModelIndex(), 0, 0);
                m_RecentDirectories.pop_front();
                endRemoveRows();
                m_DirectoriesSet.remove(directoryToRemove);
            }

            added = true;
        }

        return added;
    }

    QVariant RecentDirectoriesModel::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if (row < 0 || row >= m_RecentDirectories.length()) return QVariant();
        if (role == Qt::DisplayRole) { return m_RecentDirectories.at(index.row()); }
        return QVariant();
    }
}

