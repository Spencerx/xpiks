/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "filescleanup.h"

#include <algorithm>

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QtDebug>

#include "Common/logging.h"

namespace Maintenance {
    bool operator <(const FilesCleanup::CleanupCandidate &arg1, const FilesCleanup::CleanupCandidate &arg2) {
        return arg1.m_AgeDays < arg2.m_AgeDays ||
               ((arg1.m_AgeDays == arg2.m_AgeDays) && (arg1.m_SizeBytes < arg2.m_SizeBytes));
    }

    QDateTime getDateFromName(const QString &name) {
        QString createDateStr = name.mid(9, 8);
        return QDateTime::fromString(createDateStr, "ddMMyyyy");
    }

    void removeFile(const QString &fileNameFull) {
        QFile file(fileNameFull);

        if (file.exists()) {
            LOG_DEBUG << "Removing file " << fileNameFull;
            bool removed = false;

            try {
                removed = file.remove();
            } catch(...) {
                LOG_WARNING << "Exception while removing a file" << fileNameFull;
                removed = false;
            }

            if (!removed) {
                LOG_WARNING << "Failed to delete file " << fileNameFull;
            }
        }
    }

    void removeFiles(const QVector<FilesCleanup::CleanupCandidate> &files) {
        LOG_INFO << files.size() << "files to delete";
        for (auto &f: files) {
            removeFile(f.m_Filepath);
        }
    }

    QVector<FilesCleanup::CleanupCandidate> findCleanupCandidates(const QString &rootDir,
                                                                  const QStringList &filters,
                                                                  const QSet<QString> &ignoreList) {
        QDirIterator it(rootDir, filters, QDir::Files);
        QDateTime currentTime = QDateTime::currentDateTime();
        QVector<FilesCleanup::CleanupCandidate> candidates;

        while (it.hasNext()) {
            QString fileNameFull = it.next();
            if (ignoreList.contains(fileNameFull)) {
                LOG_DEBUG << "Skipping" << fileNameFull;
                continue;
            }

            QFileInfo fileInfo(fileNameFull);
            qint64 fileSize = fileInfo.size();
            QString fileName = fileInfo.fileName();

            QDateTime createTime = getDateFromName(fileName);
            qint64 deltaTimeDays = createTime.daysTo(currentTime);

            candidates.append({
                fileNameFull, // m_FilePath
                fileSize, // m_SizeBytes
                deltaTimeDays, // m_AgeDays
            });
        }

        LOG_INFO << "Found" << candidates.size() << "candidates(s)";
        return candidates;
    }

    // candidates vector is supposed to be sorted by age and then by size
    QVector<FilesCleanup::CleanupCandidate> filterCleanupCandidates(const QVector<FilesCleanup::CleanupCandidate> &candidates,
                                                                    qint64 overallSizeBytes,
                                                                    int maxFiles,
                                                                    qint64 maxDaysOld,
                                                                    qint64 maxSizeBytes) {
        const int size = candidates.size();
        QVector<FilesCleanup::CleanupCandidate> filesToDelete;
        filesToDelete.reserve(size);

        for (int i = size - 1; i >= 0; --i) {
            auto &info = candidates.at(i);

            const bool tooMany = (i >= maxFiles);
            const bool tooOld = (info.m_AgeDays >= maxDaysOld);
            const bool tooBig = (overallSizeBytes >= maxSizeBytes);

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

        return filesToDelete;
    }

    FilesCleanup::FilesCleanup(const QString &rootDir,
                               const QStringList &filters,
                               const QSet<QString> &ignoreList):
        FilesCleanup(findCleanupCandidates(rootDir, filters, ignoreList))
    {
    }

    FilesCleanup::FilesCleanup(QVector<FilesCleanup::CleanupCandidate> &&candidates):
        m_Candidates(std::move(candidates))
    {
        m_OverallSizeBytes = 0;
        for (auto &c: m_Candidates) { m_OverallSizeBytes += c.m_SizeBytes; }
        std::sort(m_Candidates.begin(), m_Candidates.end());
    }

    void FilesCleanup::Run(int maxFiles, qint64 maxDaysOld, qint64 maxSizeBytes) {
        LOG_DEBUG << "#";
        auto files = DryRun(maxFiles, maxDaysOld, maxSizeBytes);
        removeFiles(files);
    }

    QVector<FilesCleanup::CleanupCandidate> FilesCleanup::DryRun(int maxFiles, qint64 maxDaysOld, qint64 maxSizeBytes) {
        return filterCleanupCandidates(m_Candidates,
                                       m_OverallSizeBytes,
                                       maxFiles,
                                       maxDaysOld,
                                       maxSizeBytes);
    }
}
