/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QDirIterator>
#include "../Helpers/logger.h"
#include "logscleanupjobitem.h"

namespace Maintenance {
    bool operator <(const FileInfoHolder &arg1, const FileInfoHolder &arg2) {
        return arg1.m_AgeDays < arg2.m_AgeDays ||
                ((arg1.m_AgeDays == arg2.m_AgeDays) && (arg1.m_SizeBytes < arg2.m_SizeBytes));
    }

    LogsCleanupJobItem::LogsCleanupJobItem()
    {
        QString appDataPath = XPIKS_USERDATA_PATH;
        m_LogFileDir = QDir::cleanPath(appDataPath + QDir::separator() + "logs");
    }

    void LogsCleanupJobItem::processJob() {
        LOG_DEBUG << "#";
        doCleanLogs();
    }

    void LogsCleanupJobItem::doCleanLogs() {
        QVector<FileInfoHolder> logFiles;

        qint64 overallSizeBytes = findLogFiles(m_LogFileDir, logFiles);
        std::sort(logFiles.begin(), logFiles.end());

        QVector<FileInfoHolder> filesToDelete;
        getFilesToDelete(logFiles, overallSizeBytes, filesToDelete);

        deleteLogsFilesFromList(filesToDelete);
    }

    // logFiles vector is supposed to be sorted by age and then by size
    void LogsCleanupJobItem::getFilesToDelete(const QVector<FileInfoHolder> &logFiles, qint64 overallSizeBytes,
                          QVector<FileInfoHolder> &filesToDelete) {
        int size = logFiles.size();

        for (int i = size - 1; i >= 0; --i) {
            const FileInfoHolder &info = logFiles.at(i);

            bool tooMany = (i >= MAX_LOGS_NUMBER);
            bool tooOld = (info.m_AgeDays >= MAX_LOGS_AGE_DAYS);
            bool tooBig = (overallSizeBytes >= MAX_LOGS_SIZE_BYTES);

            if (tooMany || tooOld || tooBig) {
#ifndef CORE_TESTS
                LOG_DEBUG << info.m_Filepath << "too many:" << tooMany << "| too old:" << tooOld << "| too big:" << tooBig;
#endif
                filesToDelete.append(info);
                overallSizeBytes -= info.m_SizeBytes;
                Q_ASSERT(overallSizeBytes >= 0);
            } else {
                break;
            }
        }
    }

    qint64 LogsCleanupJobItem::findLogFiles(const QString &logsDir, QVector<FileInfoHolder> &logFiles) {
        Helpers::Logger &logger = Helpers::Logger::getInstance();
        QString logFilePath = logger.getLogFilePath();
        QDirIterator it(logsDir, QStringList() << "xpiks-qt-*.log", QDir::Files);
        QDateTime currentTime = QDateTime::currentDateTime();
        qint64 logsSizeBytes = 0;

        while (it.hasNext()) {
            QString fileNameFull = it.next();

            if (fileNameFull == logFilePath) {
                continue;
            }

            QFileInfo fileInfo(fileNameFull);
            qint64 fileSize = fileInfo.size();
            QString fileName = fileInfo.fileName();

            logsSizeBytes += fileSize;
            QDateTime createTime = getDateFromName(fileName);
            int deltaTimeDays = createTime.daysTo(currentTime);

            logFiles.append({
                             fileNameFull, // m_FilePath
                             fileSize, // m_SizeBytes
                             deltaTimeDays, // m_AgeDays
                         });
        }

        LOG_INFO << "Found" << logFiles.size() << "log file(s)";

        return logsSizeBytes;
    }

    void LogsCleanupJobItem::deleteLogsFilesFromList(const QVector<FileInfoHolder> &files) {
        LOG_INFO << files.size() << "logs to delete";
        int size = files.size();

        for (int i = 0; i < size; i++) {
            deleteLogFile(files[i].m_Filepath);
        }
    }

    void LogsCleanupJobItem::deleteLogFile(const QString &fileNameFull) {
        QFile file(fileNameFull);

        if (file.exists()) {
            LOG_DEBUG << "Removing log file " << fileNameFull;
            bool removed = false;

            try {
                removed = file.remove();
            } catch(...) {
                removed = false;
            }

            if (!removed) {
                LOG_WARNING << "Failed to delete file " << fileNameFull;
            }
        }
    }

    QDateTime LogsCleanupJobItem::getDateFromName(const QString &name) {
        QString createDateStr = name.mid(9, 8);
        return QDateTime::fromString(createDateStr, "ddMMyyyy");
    }
}
