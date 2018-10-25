/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <QApplication>
#include <QByteArray>
#include <QChar>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QList>
#include <QMetaType>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlImageProviderBase>
#include <QQuickWindow>
#include <QStaticStringData>
#include <QString>
#include <QSysInfo>
#include <QUrl>
#include <QtDebug>
#include <QtGlobal>

#include <vendors/chillout/src/chillout/chillout.h>

#include "Common/defines.h"
#include "Common/flags.h"
#include "Common/isystemenvironment.h"
#include "Common/logging.h"
#include "Common/systemenvironment.h"
#include "Common/version.h"
#include "Connectivity/curlinithelper.h"
#include "Helpers/constants.h"
#include "Helpers/globalimageprovider.h"
#include "Helpers/logger.h"
#include "Helpers/runguard.h"
#include "Models/Connectivity/proxysettings.h"
#include "Plugins/uiprovider.h"
#include "QMLExtensions/cachingimageprovider.h"
#include "QMLExtensions/imagecachingservice.h"
#include "xpiksapp.h"

class QQmlContext;

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

#ifndef Q_OS_LINUX
#include <QProcess>
#endif

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);
    QString logLine = qFormatLogMessage(type, context, msg);

    Helpers::Logger &logger = Helpers::Logger::getInstance();
    logger.log(logLine);

    if (type == QtFatalMsg) {
        logger.flush();
        abort();
    }
}

void initQSettings() {
    QCoreApplication::setOrganizationName(Constants::ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(Constants::ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(Constants::APPLICATION_NAME);
    QString appVersion(STRINGIZE(BUILDNUMBER));
    QCoreApplication::setApplicationVersion(XPIKS_VERSION_STRING " " STRINGIZE(XPIKS_VERSION_SUFFIX) " - " +
                                            appVersion.left(10));
}

void setHighDpiEnvironmentVariable() {
#ifdef Q_OS_WIN
    static const char ENV_VAR_QT_DEVICE_PIXEL_RATIO[] = "QT_DEVICE_PIXEL_RATIO";
    if (!qEnvironmentVariableIsSet(ENV_VAR_QT_DEVICE_PIXEL_RATIO)
            && !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
            && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
            && !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }
#endif
}

QString getRunGuardName() {
#ifdef QT_NO_DEBUG
    QString runGuardName = "xpiks_";
#else
    QString runGuardName = "xpiks-debug_";
#endif

    QString username = QString::fromLocal8Bit(qgetenv("USER"));
    if (username.isEmpty()) {
        username = QString::fromLocal8Bit(qgetenv("USERNAME"));
    }

    username.remove(QChar::Space);

    return (runGuardName + username);
}

void initCrashRecovery(Common::ISystemEnvironment &environment) {
    auto &chillout = Debug::Chillout::getInstance();
    QString crashesDirPath = QDir::toNativeSeparators(environment.path({Constants::CRASHES_DIR}));
#ifdef Q_OS_WIN
    chillout.init(L"xpiks", crashesDirPath.toStdWString());
#else
    chillout.init("xpiks", crashesDirPath.toStdString());
#endif
    Helpers::Logger &logger = Helpers::Logger::getInstance();

    chillout.setBacktraceCallback([&logger](const char * const stackTrace) {
        logger.emergencyLog(stackTrace);
    });

#if defined(Q_OS_WIN)
    QString recoveryApp = "Recoverty.exe";
    QStringList recoveryArgs = QStringList() << "Xpiks.exe" << "--recovery";
#elif defined(Q_OS_MAC)
    QString recoveryApp = "open";
    QString xpiksBundlePath = QCoreApplication::applicationFilePath();
    xpiksBundlePath.truncate(xpiksBundlePath.lastIndexOf(".app") + 4);
    LOG_DEBUG << "Path to Xpiks bundle is" << xpiksBundlePath;
    QString recovertyPath = QDir::cleanPath(xpiksBundlePath + "/Contents/MacOS/Recoverty.app");
    QStringList recoveryArgs = QStringList() << recovertyPath << "--args"
                                             << "open" <<  xpiksBundlePath << "--args" << "--recovery";
#else
    QString xpiksDirPath = QCoreApplication::applicationDirPath();
    std::string recoveryApp = QDir::cleanPath(xpiksDirPath + "/recoverty/Recoverty").toStdString();
    std::vector<std::string> recoveryArgs = {
        QCoreApplication::applicationFilePath().toStdString(),
        std::string("--recovery") };
#endif

    chillout.setCrashCallback([&logger, &chillout, recoveryApp, recoveryArgs]() {
        chillout.backtrace();
        logger.emergencyFlush();

#ifndef Q_OS_LINUX
        QProcess::startDetached(recoveryApp, recoveryArgs);
#else
        // Xpiks runs in AppImage and if Xpiks dies there's no way
        // Recoverty can start and even restart Xpiks later
        // therefore need to replace it's process using execl()
        execl(recoveryApp.c_str(), recoveryApp.c_str(),
              recoveryArgs[0].c_str(), recoveryArgs[1].c_str(), (char*)nullptr);
#endif

#ifdef Q_OS_WIN
        chillout.createCrashDump(Debug::CrashDumpNormal);
#endif
    });
}

int main(int argc, char *argv[]) {
    const QString runGuardName = getRunGuardName();
    Helpers::RunGuard guard(runGuardName);
    if (!guard.tryToRun()) {
        std::cerr << "Xpiks is already running";
        return -1;
    }

    // will call curl_global_init and cleanup
    Connectivity::CurlInitHelper curlInitHelper;
    Q_UNUSED(curlInitHelper);

    setHighDpiEnvironmentVariable();

    qRegisterMetaTypeStreamOperators<Models::ProxySettings>("ProxySettings");
    qRegisterMetaType<Common::SpellCheckFlags>("Common::SpellCheckFlags");
    initQSettings();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    qSetMessagePattern("%{time hh:mm:ss.zzz} %{type} T#%{threadid} %{function} - %{message}");
#endif

    qInstallMessageHandler(myMessageHandler);

    // ----------------------------------------------
    QApplication app(argc, argv);
    Common::SystemEnvironment environment(app.arguments());
    environment.ensureSystemDirectoriesExist();
    initCrashRecovery(environment);
    // ----------------------------------------------

#ifdef WITH_LOGS
    {
        QString time = QDateTime::currentDateTimeUtc().toString("ddMMyyyy-hhmmss-zzz");
        QString logFilename = QString("xpiks-qt-%1.log").arg(time);
        QString logFilePath = environment.path({Constants::LOGS_DIR, logFilename});
        Helpers::Logger &logger = Helpers::Logger::getInstance();
        logger.setLogFilePath(logFilePath);
        logger.setMemoryOnly(environment.getIsInMemoryOnly());
    }
#endif

    XpiksApp xpiks(environment);
    xpiks.startLogging();

    LOG_INFO << "Log started. Today is" << QDateTime::currentDateTimeUtc().toString("dd.MM.yyyy");
    LOG_INFO << "Xpiks" << XPIKS_FULL_VERSION_STRING << "-" << STRINGIZE(BUILDNUMBER);
    LOG_INFO << QSysInfo::productType() << QSysInfo::productVersion() << QSysInfo::currentCpuArchitecture();
    LOG_INFO << "Working directory of Xpiks is:" << QDir::currentPath();

    xpiks.initialize();

    QQmlApplicationEngine engine;
    auto &imageCachingService = xpiks.getImageCachingService();
    Helpers::GlobalImageProvider *globalProvider = new Helpers::GlobalImageProvider(QQmlImageProviderBase::Image);
    QMLExtensions::CachingImageProvider *cachingProvider = new QMLExtensions::CachingImageProvider(QQmlImageProviderBase::Image);
    cachingProvider->setImageCachingService(&imageCachingService);

    QQmlContext *rootContext = engine.rootContext();
    xpiks.setupUI(rootContext);

    engine.addImageProvider("global", globalProvider);
    engine.addImageProvider("cached", cachingProvider);

    LOG_DEBUG << "About to load main view...";
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    LOG_DEBUG << "Main view loaded";

    auto &uiProvider = xpiks.getUIProvider();
    uiProvider.setQmlEngine(&engine);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().at(0));
    imageCachingService.setScale(window->effectiveDevicePixelRatio());
    LOG_INFO << "Effective pixel ratio:" << window->effectiveDevicePixelRatio();

    xpiks.setupWindow(window);
    xpiks.start();

    return app.exec();
}
