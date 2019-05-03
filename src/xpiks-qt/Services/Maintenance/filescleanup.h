/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef FILESCLEANUP_H
#define FILESCLEANUP_H

#include <QVector>

namespace Maintenance {
    class FilesCleanup
    {
    public:
        struct CleanupCandidate {
            QString m_Filepath;
            qint64 m_SizeBytes;
            qint64 m_AgeDays;
        };
    public:
        FilesCleanup(const QString &rootDir,
                     const QStringList &filters,
                     const QSet<QString> &ignoreList);
        FilesCleanup(QVector<CleanupCandidate> &&candidates);

    public:
        void Run(int maxFiles, qint64 maxDaysOld, qint64 maxSizeBytes);
        QVector<CleanupCandidate> DryRun(int maxFiles, qint64 maxDaysOld, qint64 maxSizeBytes);

    private:
        QVector<CleanupCandidate> m_Candidates;
        qint64 m_OverallSizeBytes;
    };
}

#endif // FILESCLEANUP_H
