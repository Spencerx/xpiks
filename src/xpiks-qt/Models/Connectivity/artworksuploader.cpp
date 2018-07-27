/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworksuploader.h"
#include <QtConcurrent>
#include <QFileInfo>
#include <QQmlEngine>
#include "uploadinforepository.h"
#include "uploadinfo.h"
#include <Common/defines.h>
#include <Artworks/artworkmetadata.h>
#include <Artworks/imageartwork.h>
#include <Artworks/artworkssnapshot.h>
#include <Models/settingsmodel.h>
#include <Helpers/ziphelper.h>
#include <Helpers/filehelpers.h>
#include <Connectivity/iftpcoordinator.h>
#include <Connectivity/testconnection.h>
#include <Connectivity/ftphelpers.h>
#include <Connectivity/analyticsuserevent.h>
#include <uploadcontext.h>

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
    }

    ArtworksUploader::~ArtworksUploader() {
        if (m_TestingCredentialWatcher != nullptr) {
            delete m_TestingCredentialWatcher;
        }
    }

    void ArtworksUploader::setFtpCoordinator(const std::shared_ptr<libxpks::net::FtpCoordinator> &ftpCoordinator) {
        auto *coordinator = ftpCoordinator.get();
        QObject::connect(coordinator, &libxpks::net::FtpCoordinator::uploadStarted, this, &ArtworksUploader::onUploadStarted);
        QObject::connect(coordinator, &libxpks::net::FtpCoordinator::uploadFinished, this, &ArtworksUploader::allFinished);
        QObject::connect(coordinator, &libxpks::net::FtpCoordinator::overallProgressChanged, this, &ArtworksUploader::uploaderPercentChanged);

        m_TestingCredentialWatcher = new QFutureWatcher<Connectivity::ContextValidationResult>(this);
        QObject::connect(m_TestingCredentialWatcher, SIGNAL(finished()), SLOT(credentialsTestingFinished()));
        QObject::connect(coordinator, &libxpks::net::FtpCoordinator::transferFailed,
                         &m_UploadWatcher, &Connectivity::UploadWatcher::reportUploadErrorHandler);

        m_FtpCoordinator = std::dynamic_pointer_cast<Connectivity::IFtpCoordinator>(ftpCoordinator);
    }

    void ArtworksUploader::setPercent(double value) {
        if (fabs(m_Percent - value) > PERCENT_EPSILON) {
            m_Percent = value;
            emit percentChanged();
        }
    }

    void ArtworksUploader::setInProgress(bool value) {
        if (m_IsInProgress != value) {
            m_IsInProgress = value;
            emit inProgressChanged();
        }
    }

    void ArtworksUploader::setHasErrors(bool value) {
        if (m_HasErrors != value) {
            m_HasErrors = value;
            emit hasErrorsChanged();
        }
    }

    void ArtworksUploader::onUploadStarted() {
        LOG_DEBUG << "#";
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
        Connectivity::ContextValidationResult result = m_TestingCredentialWatcher->result();
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

        m_TestingCredentialWatcher->setFuture(QtConcurrent::run(Connectivity::isContextValid, context));
    }

    bool ArtworksUploader::needCreateArchives() const {
        bool needCreate = false;

        if (m_UploadInfos.isZippingRequired()) {
            auto &snapshot = this->getArtworksSnapshot();
            for (auto &locker: snapshot.getRawData()) {
                auto *artwork = locker->getArtworkMetadata();
                Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork *>(artwork);
                if (image == NULL || !image->hasVectorAttached()) { continue; }

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

        std::vector<std::shared_ptr<Models::UploadInfo> > selectedInfos = std::move(m_UploadInfos.retrieveSelectedUploadInfos());

        m_UploadInfos.resetPercents();
        m_UploadInfos.updatePercentages();

        m_FtpCoordinator->uploadArtworks(snapshot, selectedInfos);
        sendMessage(Connectivity::UserAction::Upload);
    }

    bool ArtworksUploader::removeUnavailableItems() {
        LOG_DEBUG << "#";

        auto &artworksListOld = getArtworksSnapshot();
        Artworks::ArtworksSnapshot::Container artworksListNew;

        const size_t size = artworksListOld.size();
        for (size_t i = 0; i < size; ++i) {
            auto &item = artworksListOld.at(i);

            if (!item->getArtworkMetadata()->isUnavailable()) {
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

    QObject *ArtworksUploader::getUploadWatcher() {
        auto *model = &m_UploadWatcher;
        QQmlEngine::setObjectOwnership(model, QQmlEngine::CppOwnership);

        return model;
    }
}
