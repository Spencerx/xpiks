/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>

#include <QtQml>
#include <QScreen>
#include <QtDebug>
#include <QDateTime>
#include <QSettings>
#include <QTextStream>
#include <QQmlContext>
#include <QApplication>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QQmlApplicationEngine>
#include <QDesktopWidget>

// -------------------------------------

#include "Common/defines.h"
#include "Common/version.h"
#include "Common/systemenvironment.h"
#include "xpiksapp.h"
#include "Helpers/globalimageprovider.h"
#include "Helpers/logger.h"
#include "Helpers/runguard.h"
#include "Helpers/clipboardhelper.h"
#include "Connectivity/curlinithelper.h"
#include "QMLExtensions/cachingimageprovider.h"
#include "QMLExtensions/folderelement.h"
#include "QMLExtensions/triangleelement.h"

#ifdef Q_OS_WIN
// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

#include "ShellScalingApi.h"
#endif

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    QString logLine = qFormatLogMessage(type, context, msg);
#else
    QString msgType;
    switch (type) {
        case QtDebugMsg:
            msgType = "debug";
            break;
        case QtWarningMsg:
            msgType = "warning";
            break;
        case QtCriticalMsg:
            msgType = "critical";
            break;
        case QtFatalMsg:
            msgType = "fatal";
            break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 1))
        case QtInfoMsg:
            msgType = "info";
            break;
#endif
    }

    // %{time hh:mm:ss.zzz} %{type} T#%{threadid} %{function} - %{message}
    QString time = QDateTime::currentDateTimeUtc().toString("hh:mm:ss.zzz");
    QString logLine = QString("%1 %2 T#%3 %4 - %5")
                          .arg(time).arg(msgType)
                          .arg(0).arg(context.function)
                          .arg(msg);
#endif

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

#if QT_VERSION >= QT_VERSION_CHECK(5,6,0)
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#else
    qputenv("QT_DEVICE_PIXEL_RATIO", QByteArray("1"));
#endif

    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
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

    qmlRegisterType<Helpers::ClipboardHelper>("xpiks", 1, 0, "ClipboardHelper");
    qmlRegisterType<QMLExtensions::TriangleElement>("xpiks", 1, 0, "TriangleElement");
    qmlRegisterType<QMLExtensions::FolderElement>("xpiks", 1, 0, "FolderElement");

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

    auto *uiProvider = xpiks.getUIProvider();
    uiProvider->setQmlEngine(&engine);
    QQuickWindow *window = qobject_cast<QQuickWindow *>(engine.rootObjects().at(0));
    imageCachingService.setScale(window->effectiveDevicePixelRatio());

    QScreen *screen = window->screen();
    QObject::connect(window, &QQuickWindow::screenChanged, &imageCachingService, &QMLExtensions::ImageCachingService::screenChangedHandler);
    QObject::connect(screen, &QScreen::logicalDotsPerInchChanged, &imageCachingService, &QMLExtensions::ImageCachingService::dpiChanged);
    QObject::connect(screen, &QScreen::physicalDotsPerInchChanged, &imageCachingService, &QMLExtensions::ImageCachingService::dpiChanged);

    uiProvider->setRoot(window->contentItem());

    xpiks.start();

    return app.exec();
}
