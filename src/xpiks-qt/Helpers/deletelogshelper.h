/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DELETELOGSHELPER_H
#define DELETELOGSHELPER_H

#include <QString>
#include <QVector>

#define MAX_LOGS_SIZE_MB 10
#define MAX_LOGS_SIZE_BYTES (MAX_LOGS_SIZE_MB * 1024 * 1024)
#define MAX_LOGS_AGE_DAYS 60
#define MAX_LOGS_NUMBER 100

namespace Helpers {
    struct FileInfoHolder {
        QString m_Filepath;
        qint64 m_SizeBytes;
        int m_AgeDays;
    };

    void performCleanLogsAsync();
    void getFilesToDelete(const QVector<FileInfoHolder> &logFiles, qint64 overallSizeBytes,
                          QVector<FileInfoHolder> &filesToDelete);
    bool operator <(const FileInfoHolder &arg1, const FileInfoHolder &arg2);
}
#endif // DELETELOGSHELPER_H
