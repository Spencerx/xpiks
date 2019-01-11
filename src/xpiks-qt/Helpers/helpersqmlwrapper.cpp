/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helpersqmlwrapper.h"

#include <QChar>
#include <QFileInfo>
#include <QProcess>
#include <QStringList>
#include <Qt>
#include <QtDebug>
#include <QtGlobal>

#include "Common/logging.h"
#include "Helpers/filehelpers.h"
#include "Helpers/keywordshelpers.h"
#include "Helpers/logger.h"
#include "QMLExtensions/colorsmodel.h"

#ifdef Q_OS_WIN
#include <QDir>
#include <QQuickWindow>
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

namespace Helpers {
    HelpersQmlWrapper::HelpersQmlWrapper(Common::ISystemEnvironment &environment, QMLExtensions::ColorsModel &colorsModel):
        m_Environment(environment),
        m_ColorsModel(colorsModel)
    {
#if defined(Q_OS_WIN) && !defined(INTEGRATION_TESTS) && !defined(UI_TESTS)
        m_WinTaskbarButtonApplicable = QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7;
        if (m_WinTaskbarButtonApplicable) {
            m_TaskbarButton = new QWinTaskbarButton(this);
        } else {
            m_TaskbarButton = NULL;
        }
#endif
    }

    bool HelpersQmlWrapper::isKeywordValid(const QString &keyword) const {
        return isValidKeyword(keyword);
    }

    QString HelpersQmlWrapper::sanitizeKeyword(const QString &keyword) const {
        return doSanitizeKeyword(keyword);
    }

    void HelpersQmlWrapper::revealLogFile() {
        LOG_DEBUG << "#";
        QString logFilePath = Logger::getInstance().getLogFilePath();
        HelpersQmlWrapper::revealFile(logFilePath);
    }

    void HelpersQmlWrapper::revealArtworkFile(const QString &path) {
        HelpersQmlWrapper::revealFile(path);
    }

    void Helpers::HelpersQmlWrapper::reportOpen() {
        //xpiks()->reportUserAction(Connectivity::UserAction::Open);
    }

    void HelpersQmlWrapper::setProgressIndicator(QQuickWindow *window) {
#ifdef Q_OS_WIN
        if (!m_WinTaskbarButtonApplicable) { return; }
        m_TaskbarButton->setWindow(window);
#else
        Q_UNUSED(window);
#endif
    }

    void HelpersQmlWrapper::turnTaskbarProgressOn() {
#if defined(Q_OS_WIN) && !defined(UI_TESTS)
        if (!m_WinTaskbarButtonApplicable) { return; }
        LOG_DEBUG << "Turning on taskbar button in Windows";
        QWinTaskbarProgress *progress = m_TaskbarButton->progress();
        progress->setVisible(true);
        progress->setRange(0, 100);
        progress->setValue(0);
#endif
    }

    void HelpersQmlWrapper::setTaskbarProgress(double value) {
#if defined(Q_OS_WIN) && !defined(UI_TESTS)
        if (!m_WinTaskbarButtonApplicable) { return; }
        LOG_DEBUG << value;
        QWinTaskbarProgress *progress = m_TaskbarButton->progress();
        progress->setValue((int)value);
#else
        Q_UNUSED(value);
#endif
    }

    void HelpersQmlWrapper::turnTaskbarProgressOff() {
#if defined(Q_OS_WIN) && !defined(UI_TESTS)
        if (!m_WinTaskbarButtonApplicable) { return; }
        LOG_DEBUG << "Turning off taskbar button in Windows";
        QWinTaskbarProgress *progress = m_TaskbarButton->progress();
        progress->setVisible(false);
#endif
    }

    bool HelpersQmlWrapper::isVector(const QString &path) const {
        return path.endsWith("eps", Qt::CaseInsensitive) ||
                path.endsWith("ai", Qt::CaseInsensitive);
    }

    bool HelpersQmlWrapper::isVideo(const QString &path) const {
        return Helpers::couldBeVideo(path);
    }

    QString HelpersQmlWrapper::toImagePath(const QString &path) const {
        return Helpers::getImagePath(path);
    }

    QString HelpersQmlWrapper::getAssetForTheme(const QString &assetName, int themeIndex) const {
        QString themeName = m_ColorsModel.getThemeName(themeIndex);
        themeName.remove(QChar::Space);
        LOG_VERBOSE << assetName << themeName;
        QString result = QString("qrc:/Graphics/%1/%2").arg(themeName.toLower()).arg(assetName);
        return result;
    }

    /*QObject *HelpersQmlWrapper::getLogsModel() {
        Models::LogsModel *model = m_CommandManager->getLogsModel();
        QQmlEngine::setObjectOwnership(model, QQmlEngine::CppOwnership);
        return model;
    }*/

    void HelpersQmlWrapper::revealFile(const QString &path) {
#ifdef Q_OS_MAC
        QStringList args;
        args << "-e";
        args << "tell application \"Finder\"";
        args << "-e";
        args << "activate";
        args << "-e";
        args << "select POSIX file \"" + path + "\"";
        args << "-e";
        args << "end tell";
        QProcess::startDetached("osascript", args);
#endif

#ifdef Q_OS_WIN
        QStringList args;
        args << "/select," << QDir::toNativeSeparators(path);
        QProcess::startDetached("explorer", args);
#endif

#ifdef Q_OS_LINUX
        QStringList args;
        args << QFileInfo(path).absolutePath();
        QProcess::startDetached("xdg-open", args);
#endif
    }
}
