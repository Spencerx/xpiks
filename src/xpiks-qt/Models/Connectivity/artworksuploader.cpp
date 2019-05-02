/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworksuploader.h"

#include <cmath>
#include <utility>

#include <QException>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QQmlEngine>
#include <QtConcurrentRun>
#include <QtDebug>
#include <QtGlobal>

#include <vendors/libxpks/ftpcoordinator.h>
#include <vendors/libxpks/uploadcontext.h>

#include "Artworks/artworkmetadata.h"
#include "Artworks/artworkssnapshot.h"
#include "Artworks/imageartwork.h"
#include "Common/logging.h"
#include "Connectivity/analyticsuserevent.h"
#include "Connectivity/iftpcoordinator.h"
#include "Connectivity/testconnection.h"
#include "Connectivity/uploadwatcher.h"
#include "Helpers/filehelpers.h"
#include "Models/Connectivity/uploadinfo.h"
#include "Models/Connectivity/uploadinforepository.h"
#include "Models/settingsmodel.h"

namespace Models {
    ArtworksUploader::ArtworksUploader(Common::ISystemEnvironment &environment,
                                     UploadInfoRepository &uploadInfoRepository,
                                     SettingsModel &settingsModel,
                                     QObject *parent):
        QObject(parent),
        m_Environment(environment),
        m_TestingCredentialWatcher(nullptr),
        m_UploadInfos(uploadInfoRepository),
        m_SettingsModel(settingsModel),
        m_Percent(0),
        m_IsInProgress(false),
        m_HasErrors(false)
    {
        QObject::connect(&m_TestingCredentialWatcher, &QFutureWatcher<Connectivity::ContextValidationResult>::finished,
                         this, &ArtworksUploader::credentialsTestingFinished);
    }

    void ArtworksUploader::setFtpCoordinator(const std::shared_ptr<libxpks::net::FtpCoordinator> &ftpCoordinator) {
        LOG_DEBUG << "#";
        auto *coordinator = ftpCoordinator.get();
        QObject::connect(coordinator, &libxpks::net::FtpCoordinator::uploadStarted, this, &ArtworksUploader::onUploadStarted);
        QObject::connect(coordinator, &libxpks::net::FtpCoordinator::uploadFinished, this, &ArtworksUploader::allFinished);
        QObject::connect(coordinator, &libxpks::net::FtpCoordinator::overallProgressChanged, this, &ArtworksUploader::uploaderPercentChanged);

        QObject::connect(coordinator, &libxpks::net::FtpCoordinator::transferFailed,
                         &m_UploadWatcher, &Connectivity::UploadWatcher::reportUploadErrorHandler);

        m_FtpCoordinator = ftpCoordinator;
    }

    void ArtworksUploader::setPercent(double value) {
        LOG_INFO << value;
        if (fabs(m_Percent - value) > PERCENT_EPSILON) {
            m_Percent = value;
            emit percentChanged();
        }
    }

    void ArtworksUploader::setInProgress(bool value) {
        LOG_INFO << value;
        if (m_IsInProgress != value) {
            m_IsInProgress = value;
            emit inProgressChanged();
        }
    }

    void ArtworksUploader::setHasErrors(bool value) {
        LOG_INFO << value;
        if (m_HasErrors != value) {
            m_HasErrors = value;
            emit hasErrorsChanged();
        }
    }

    void ArtworksUploader::onUploadStarted() {
        LOG_DEBUG << "#";
        Q_ASSERT(!getInProgress());
        setHasErrors(false);
        setInProgress(true);
        setPercent(0);
        emit startedProcessing();
    }

    void ArtworksUploader::allFinished(bool anyError) {
        LOG_INFO << "anyError =" << anyError;
        setHasErrors(anyError);
        setPercent(100);
        setInProgress(false);
        emit finishedProcessing();
    }

    void ArtworksUploader::credentialsTestingFinished() {
        Connectivity::ContextValidationResult result = m_TestingCredentialWatcher.result();
        emit credentialsChecked(result.m_Result, result.m_Host);
    }

    void ArtworksUploader::uploaderPercentChanged(double percent) {
        setPercent(percent);
        LOG_DEBUG << "Overall progress =" << percent;

        m_UploadInfos.updatePercentages();
    }

    void ArtworksUploader::setArtworks(Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "#";
        m_ArtworksSnapshot = std::move(snapshot);
        emit itemsCountChanged();
    }

    void ArtworksUploader::handleMessage(const UnavailableFilesMessage &) {
        LOG_DEBUG << "#";
        removeUnavailableItems();
    }

    void ArtworksUploader::uploadArtworks() {
        doUploadArtworks(m_ArtworksSnapshot);
    }

    void ArtworksUploader::checkCredentials(const QString &host, const QString &username,
                                           const QString &password, bool disablePassiveMode, bool disableEPSV) {
        if (host.isEmpty() || username.isEmpty()) {
            emit credentialsChecked(false, host);
            return;
        }

        auto context = std::make_shared<libxpks::net::UploadContext>();

        context->m_Host = host;
        context->m_Username = username;
        context->m_Password = password;
        context->m_TimeoutSeconds = 10;
        context->m_UsePassiveMode = !disablePassiveMode;
        context->m_UseEPSV = !disableEPSV;

        context->m_UseProxy = m_SettingsModel.getUseProxy();
        context->m_ProxySettings = &m_SettingsModel.getProxySettings();
        context->m_VerboseLogging = m_SettingsModel.getVerboseUpload();

        m_TestingCredentialWatcher.setFuture(QtConcurrent::run(Connectivity::isContextValid, context));
        sendMessage(Connectivity::EventType::CheckUploadCredentials);
    }

    bool ArtworksUploader::needCreateArchives() const {
        bool needCreate = false;

        if (m_UploadInfos.isZippingRequired()) {
            auto &snapshot = this->getArtworksSnapshot();
            for (auto &artwork: snapshot) {
                auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
                if (image == nullptr || !image->hasVectorAttached()) { continue; }

                const QString &filepath = artwork->getFilepath();
                QString archivePath = Helpers::getArchivePath(filepath);
                QFileInfo fi(archivePath);

                if (!fi.exists()) {
                    needCreate = true;
                    LOG_DEBUG << "Zip needed at least for" << archivePath;
                    break;
                }
            }
        }

        return needCreate;
    }

    void ArtworksUploader::resetModel() {
        LOG_DEBUG << "#";
        clearModel();
        resetArtworks();
    }

    void ArtworksUploader::clearModel() {
        LOG_DEBUG << "#";
        resetProgress();
        setHasErrors(false);
        m_UploadWatcher.resetModel();
    }

    void ArtworksUploader::resetProgress() {
        LOG_DEBUG << "#";
        setPercent(0);
        setInProgress(false);
    }

    void ArtworksUploader::cancelOperation() {
        LOG_DEBUG << "#";
        m_FtpCoordinator->cancelUpload();
    }

    void ArtworksUploader::resetArtworks() {
        LOG_DEBUG << "#";
        m_ArtworksSnapshot.clear();
        emit itemsCountChanged();
    }

    void ArtworksUploader::doUploadArtworks(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_INFO << snapshot.size() << "artwork(s)";
        Q_ASSERT(m_FtpCoordinator != nullptr);

        if (snapshot.empty()) { return; }
        if (!m_FtpCoordinator) {
            LOG_WARNING << "FTP coordinator is not set";
            return;
        }

        auto selectedInfos = m_UploadInfos.retrieveSelectedUploadInfos();
        LOG_DEBUG << "Found" << selectedInfos.size() << "selected upload info(s)";

        m_UploadInfos.resetPercents();
        m_UploadInfos.updatePercentages();

        m_FtpCoordinator->uploadArtworks(snapshot, selectedInfos);
        sendMessage(Connectivity::EventType::Upload);
    }

    bool ArtworksUploader::removeUnavailableItems() {
        LOG_DEBUG << "#";

        auto &artworksListOld = getArtworksSnapshot();
        Artworks::ArtworksSnapshot::Container artworksListNew;

        for (auto &item: artworksListOld) {
            if (!item->isUnavailable()) {
                artworksListNew.push_back(item);
            }
        }

        bool anyUnavailable = artworksListNew.size() != m_ArtworksSnapshot.size();
        if (anyUnavailable) {
            m_ArtworksSnapshot.set(artworksListNew);

            if (m_ArtworksSnapshot.empty()) {
                emit requestCloseWindow();
            }

            emit itemsCountChanged();
        }

        return anyUnavailable;
    }

    QString ArtworksUploader::getFtpName(const QString &stockAddress) const {
        QString result;
        auto info = m_UploadInfos.tryFindItemByHost(stockAddress);
        if (info) {
            result = info->getTitle();
        }

        return result;
    }

    QObject *ArtworksUploader::getUploadWatcherObject() {
        QObject *result = &m_UploadWatcher;
        QQmlEngine::setObjectOwnership(result, QQmlEngine::CppOwnership);
        return result;
    }

    QObject *ArtworksUploader::getUploadInfosObject() {
        QObject *result = &m_UploadInfos;
        QQmlEngine::setObjectOwnership(result, QQmlEngine::CppOwnership);
        return result;
    }
}
