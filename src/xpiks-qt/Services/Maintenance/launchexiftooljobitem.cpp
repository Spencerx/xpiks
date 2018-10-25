/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "launchexiftooljobitem.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStringList>
#include <QtDebug>
#include <QtGlobal>

#include "Common/logging.h"

#if defined(Q_OS_WIN)
#include <QStandardPaths>
#endif

#define EXIFTOOL_VERSION_TIMEOUT 3000

namespace Maintenance {
    bool tryGetExiftoolVersion(const QString &path, QString &version) {
        Q_ASSERT(!path.isEmpty());
        QProcess process;
        process.start(path, QStringList() << "-ver");
        const bool finishedInTime = process.waitForFinished(EXIFTOOL_VERSION_TIMEOUT);
        if (!finishedInTime) {
            LOG_WARNING << "Exiftool did not finish in time";
        }

        const int exitCode = process.exitCode();
        const QProcess::ExitStatus exitStatus = process.exitStatus();

        bool success = finishedInTime &&
                (exitCode == 0) &&
                (exitStatus == QProcess::NormalExit);

        LOG_DEBUG << "Exiftool exitcode =" << exitCode << "exitstatus =" << exitStatus;

        if (success) {
            version = QString::fromUtf8(process.readAll());
        } else {
            if (process.state() != QProcess::NotRunning) {
                process.kill();
                process.waitForFinished(EXIFTOOL_VERSION_TIMEOUT);
            }
        }

        return success;
    }

    QString discoverExiftool(const QString &defaultPath) {
        LOG_DEBUG << "Default path is" << defaultPath;

        QStringList possiblePaths;
        QString exiftoolPath;
        QString exiftoolVersion;

        possiblePaths << defaultPath;

#if defined(Q_OS_MAC)
        QString resourcesPath = QCoreApplication::applicationDirPath();
        resourcesPath += "/../Resources/exiftool/exiftool";
        possiblePaths << resourcesPath << "/usr/bin/exiftool" << "/usr/local/bin/exiftool";
#elif defined(Q_OS_WIN)
        possiblePaths << "c:/exiftool.exe";
        QDir downloadsDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
        possiblePaths << downloadsDir.filePath("exiftool.exe");
#elif defined(Q_OS_LINUX)
        QString resourcesPath = QCoreApplication::applicationDirPath();
        const QString exiftoolAppImagePath = QDir::cleanPath(resourcesPath + "/exiftool/exiftool");
        possiblePaths << exiftoolAppImagePath;
        possiblePaths << "/usr/bin/exiftool" << "/usr/local/bin/exiftool";
#else
        // BUMP
#endif

        possiblePaths << "exiftool";

        foreach (const QString &path, possiblePaths) {
            LOG_DEBUG << "Trying path" << path;
            if (path.isEmpty()) { continue; }

            if (!QFileInfo(path).exists()) {
                LOG_DEBUG << "File" << path << "does not exist";
            }

            if (tryGetExiftoolVersion(path, exiftoolVersion)) {
                LOG_INFO << "Exiftool version:" << exiftoolVersion;
                exiftoolPath = path;
                break;
            }
        }

        return exiftoolPath;
    }

    LaunchExiftoolJobItem::LaunchExiftoolJobItem(const QString &settingsExiftoolPath):
        m_SettingsExiftoolPath(settingsExiftoolPath)
    {
    }

    void LaunchExiftoolJobItem::processJob() {
        LOG_DEBUG << "#";
        QString recommendedPath = discoverExiftool(m_SettingsExiftoolPath);
        emit exiftoolDetected(recommendedPath);
    }
}
