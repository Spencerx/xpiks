/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "updatehelpers.h"
#include <QProcess>
#include <QStringList>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include "../Common/defines.h"

#ifdef Q_OS_OSX

void launchOSXdmg(const QString &dmgPath) {
    QStringList arguments;
    arguments << dmgPath;

    QProcess::startDetached("open", arguments);
}

#endif

#ifdef Q_OS_WIN

QString getLogFilenameForMinistaller() {
    QString time = QDateTime::currentDateTimeUtc().toString("ddMMyyyy-hhmmss-zzz");
    QString logFilename = QString("ministaller-%1.log").arg(time);
    return logFilename;
}

void launchWindowsInstaller(Common::ISystemEnvironment &environment, const QString &pathToUpdate) {
    LOG_DEBUG << "#";
    QString ministallerPath = environment.filepath("ministaller.exe");

    if (!QFileInfo(ministallerPath).exists()) {
        LOG_WARNING << "Updater not found!" << ministallerPath;
        return;
    }

    QString installerLogName = getLogFilenameForMinistaller();
    QString installerLogPath = environment.fileInDir(installerLogName, "logs");
    QStringList arguments;
    arguments << "-force-update" << "-gui" <<
                 "-install-path" << appDirPath <<
                 "-l" << installerLogPath <<
                 "-launch-exe" << "Xpiks.exe" <<
                 "-package-path" << pathToUpdate <<
                 "-stdout";

    QProcess::startDetached(ministallerPath, arguments);
}

#endif

namespace Helpers {
    void installUpdate(Common::ISystemEnvironment &environment, const QString &updatePath) {
#if defined(Q_OS_OSX)
        launchOSXdmg(updatePath);
#elif defined(Q_OS_WIN)
        launchWindowsInstaller(environment, updatePath);
#endif
    }
}
