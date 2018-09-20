/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "updatescheckerworker.h"

#define UPDATE_JSON_MAJOR_VERSION "major_version"
#define UPDATE_JSON_MINOR_VERSION "minor_version"
#define UPDATE_JSON_FIX_VERSION "fix_version"
#define UPDATE_JSON_HOTFIX_VERSION "hotfix_version"
#define UPDATE_JSON_UPDATE_URL "update_link"
#define UPDATE_JSON_CHECKSUM "checksum"

#include <QString>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtGlobal>
#include <QCryptographicHash>
#include "simplecurlrequest.h"
#include "simplecurldownloader.h"
#include "apimanager.h"
#include <Common/defines.h>
#include <Common/version.h>
#include <Models/settingsmodel.h>
#include <Models/Connectivity/proxysettings.h>
#include <Helpers/constants.h>
#include <Services/Maintenance/maintenanceservice.h>

QString fileChecksum(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm) {
    QString result;

    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(hashAlgorithm);
        if (hash.addData(&f)) {
            result = QString::fromLatin1(hash.result().toHex()).toLower();
        }
    }

    return result;
}

bool moveFile(const QString &from, const QString &to) {
    LOG_INFO << from << "->" << to;
    bool success = false;

    QFile destination(to);
    if (destination.exists()) {
        if (!destination.remove()) {
            LOG_WARNING << "Failed to remove exising file";
            return success;
        }
    }

    QFile source(from);
    if (source.exists()) {
        success = source.copy(to);
    } else {
        LOG_WARNING << "Source does not exist";
    }

    return success;
}

QString urlFilename(const QString &url) {
    QString filename;

    int pos = url.lastIndexOf(QLatin1Char('/'));
    if (pos != -1) {
        filename = url.mid(pos + 1);
    }

    return filename;
}

namespace Connectivity {
    UpdatesCheckerWorker::UpdatesCheckerWorker(Common::ISystemEnvironment &environment,
                                               Models::SettingsModel &settingsModel,
                                               Maintenance::MaintenanceService &maintenanceService,
                                               const QString &availableUpdatePath):
        m_Environment(environment),
        m_SettingsModel(settingsModel),
        m_MaintenanceService(maintenanceService),
        m_AvailableUpdatePath(availableUpdatePath)
    {
    }

    void UpdatesCheckerWorker::initWorker() {
        m_Environment.ensureDirExists(Constants::UPDATES_DIRECTORY);
        m_UpdatesDirectory = m_Environment.path({Constants::UPDATES_DIRECTORY});
    }

    void UpdatesCheckerWorker::processOneItem() {
        LOG_INFO << "Checking for updates...";

        UpdateCheckResult updateCheckResult;
        if (checkForUpdates(updateCheckResult)) {
#ifndef Q_OS_LINUX
            if (checkAvailableUpdate(updateCheckResult)) {
                LOG_INFO << "Update is already downloaded:" << m_AvailableUpdatePath;
                emit updateDownloaded(m_AvailableUpdatePath, updateCheckResult.m_Version);
            } else if (m_SettingsModel.getAutoDownloadUpdates() && !m_Environment.getIsInMemoryOnly()) {
                LOG_DEBUG << "Going to download update...";
                QString pathToUpdate;
                if (downloadUpdate(updateCheckResult, pathToUpdate)) {
                    emit updateDownloaded(pathToUpdate, updateCheckResult.m_Version);
                } else {
                    emit updateAvailable(updateCheckResult.m_UpdateURL);
                }
            } else
#endif
            {
                emit updateAvailable(updateCheckResult.m_UpdateURL);
            }
        } else {
            m_MaintenanceService.cleanupDownloadedUpdates(m_UpdatesDirectory);
        }

        emit stopped();
        emit requestFinished();

        LOG_INFO << "Updates checking loop finished";
    }

    bool UpdatesCheckerWorker::checkForUpdates(UpdateCheckResult &result) {
        bool anyUpdateAvailable = false;
        auto &apiManager = ApiManager::getInstance();
        QString queryString = apiManager.getUpdateAddr();

        Models::ProxySettings *proxySettings = m_SettingsModel.retrieveProxySettings();

        Connectivity::SimpleCurlRequest request(queryString);
        request.setProxySettings(proxySettings);

        if (request.sendRequestSync()) {
            QJsonDocument document = QJsonDocument::fromJson(request.getResponseData());

            QJsonObject jsonObject = document.object();

            if (jsonObject.contains(UPDATE_JSON_MAJOR_VERSION) &&
                    jsonObject.contains(UPDATE_JSON_MINOR_VERSION) &&
                    jsonObject.contains(UPDATE_JSON_FIX_VERSION) /*&&
                    jsonObject.contains(UPDATE_JSON_HOTFIX_VERSION)*/) {

                const int majorVersion = jsonObject.value(UPDATE_JSON_MAJOR_VERSION).toInt();
                const int minorVersion = jsonObject.value(UPDATE_JSON_MINOR_VERSION).toInt();
                const int fixVersion = jsonObject.value(UPDATE_JSON_FIX_VERSION).toInt();
                /*const int hotfixVersion = jsonObject.value(UPDATE_JSON_HOTFIX_VERSION).toInt();*/

                const int availableVersion = XPIKS_VERSION_TO_INT(majorVersion, minorVersion, fixVersion/*, hotfixVersion*/);
                const int currVersion = XPIKS_VERSION_INT;

                LOG_INFO << "Update service: available =" << availableVersion << "current =" << currVersion;

                if (availableVersion > currVersion) {
                    QString updateUrl = apiManager.getDefaultUpdateAddr();
                    if (jsonObject.contains(UPDATE_JSON_UPDATE_URL)) {
                        updateUrl = jsonObject.value(UPDATE_JSON_UPDATE_URL).toString();
                    }

                    QString checksum;
                    if (jsonObject.contains(UPDATE_JSON_CHECKSUM)) {
                        checksum = jsonObject.value(UPDATE_JSON_CHECKSUM).toString();
                    }

                    result.m_UpdateURL = updateUrl;
                    result.m_Checksum = checksum;
                    result.m_Version = availableVersion;
                    anyUpdateAvailable = true;
                }
            }
        }

        return anyUpdateAvailable;
    }

    bool UpdatesCheckerWorker::downloadUpdate(const UpdateCheckResult &updateCheckResult, QString &pathToUpdate) {
        LOG_DEBUG << "#";
        Q_ASSERT(!m_Environment.getIsInMemoryOnly());
        bool success = false;

        Connectivity::SimpleCurlDownloader downloader(updateCheckResult.m_UpdateURL);
        QObject::connect(this, &UpdatesCheckerWorker::cancelRequested,
                         &downloader, &Connectivity::SimpleCurlDownloader::cancelRequested);

        Models::ProxySettings *proxySettings = m_SettingsModel.retrieveProxySettings();
        downloader.setProxySettings(proxySettings);

        if (downloader.downloadFileSync()) {
            LOG_INFO << "Update downloaded" << downloader.getDownloadedPath();

            QString downloadedPath = downloader.getDownloadedPath();
            QString checksum = fileChecksum(downloadedPath, QCryptographicHash::Sha1);
            if (checksum == updateCheckResult.m_Checksum) {
                LOG_INFO << "Update checksum confirmed";

                QDir updatesDir(m_UpdatesDirectory);
                Q_ASSERT(updatesDir.exists());

                QString realFilename = urlFilename(updateCheckResult.m_UpdateURL);
                QString updatePath = updatesDir.filePath(realFilename);

                if (moveFile(downloader.getDownloadedPath(), updatePath)) {
                    pathToUpdate = updatePath;
                    success = true;
                } else {
                    LOG_WARNING << "Failed to move the file";
                }
            } else {
                LOG_INFO << "Checksum does not match:" << "expected" << updateCheckResult.m_Checksum << "actual" << checksum;
            }
        } else {
            LOG_WARNING << "Update download failed";
        }

        return success;
    }

    bool UpdatesCheckerWorker::checkAvailableUpdate(const UpdateCheckResult &updateCheckResult) {
        bool result = false;

        if (QFileInfo(m_AvailableUpdatePath).exists()) {
            LOG_DEBUG << m_AvailableUpdatePath << "exists";
            QString availableChecksum = fileChecksum(m_AvailableUpdatePath, QCryptographicHash::Sha1);
            if (availableChecksum == updateCheckResult.m_Checksum) {
                result = true;
            } else {
                LOG_INFO << "Existing checksum is:" << availableChecksum << "but expected" << updateCheckResult.m_Checksum;
            }
        }

        return result;
    }

    void UpdatesCheckerWorker::process() {
        initWorker();
        processOneItem();
    }
}
