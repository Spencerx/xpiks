/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uploadwatcher.h"
#include <QFileInfo>
#include "../Common/logging.h"

namespace Connectivity {
    UploadWatcher::UploadWatcher(QObject *parent):
        QAbstractListModel(parent),
        m_FailedImagesCount(0)
    {}

    void UploadWatcher::resetModel() {
        LOG_DEBUG << "Resetting UploadWatcher..";

        beginResetModel();
        m_FtpInfo.clear();
        endResetModel();

        m_FailedImagesCount = 0;
        emit failedImagesCountChanged();
    }

    int UploadWatcher::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return m_FtpInfo.size();
    }

    QVariant UploadWatcher::data(const QModelIndex &index, int role) const {
        int row = index.row();

        if (row < 0 || row >= m_FtpInfo.size()) {
            return QVariant();
        }

        auto &item = m_FtpInfo.at(row);

        switch (role) {
            case FtpAddressRole:
                return item.first;
            case FailedImagesRole:
                return item.second;
            default:
                return QVariant();
        }
    }

    QHash<int, QByteArray> UploadWatcher::roleNames() const {
        QHash<int, QByteArray> names = QAbstractListModel::roleNames();
        names[FtpAddressRole] = "ftpaddress";
        names[FailedImagesRole] = "failedimages";
        return names;
    }

    void UploadWatcher::reportUploadErrorHandler(const QString &filepath, const QString &host) {
        LOG_WARNING << "Upload failed for file [" << filepath << "] to host {" << host << "}";

        bool hostExists = false;
        int size = m_FtpInfo.size();

        for (int i = 0; i < size; i++) {
            if (m_FtpInfo[i].first == host) {
                m_FtpInfo[i].second.append(filepath);
                auto modelIndex = this->index(i);
                emit dataChanged(modelIndex, modelIndex, QVector<int>() << FailedImagesRole);
                hostExists = true;
                break;
            }
        }

        if (!hostExists) {
            LOG_INFO << "Registering new host:" << host;
            beginInsertRows(QModelIndex(), size, size);
            m_FtpInfo.append(QPair<QString, QStringList>(host, QStringList(filepath)));
            endInsertRows();
        }

        m_FailedImagesCount++;
        emit failedImagesCountChanged();
    }
}
