/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKSUPLOADER_H
#define ARTWORKSUPLOADER_H

#include <memory>

#include <QObject>
#include <QFutureWatcher>
#include <QString>

#include "Artworks/artworkssnapshot.h"
#include "Common/messages.h"
#include "Common/types.h"
#include "Connectivity/analyticsuserevent.h"
#include "Connectivity/testconnection.h"
#include "Connectivity/uploadwatcher.h"

namespace Common {
    class ISystemEnvironment;
}

namespace Models {
    class UploadInfoRepository;
}

namespace libxpks {
    namespace net {
        class FtpCoordinator;
    }
}

namespace Connectivity {
    class IFtpCoordinator;
}

namespace Models {
    class SettingsModel;
#define PERCENT_EPSILON 0.0001

    using UnavailableFilesMessage = Common::NamedType<int, Common::MessageType::UnavailableFiles>;

    class ArtworksUploader:
            public QObject,
            public Common::MessagesSource<Common::NamedType<Connectivity::UserAction>>,
            public Common::MessagesTarget<UnavailableFilesMessage>
    {
        Q_OBJECT
        Q_PROPERTY(int percent READ getUIPercent NOTIFY percentChanged)
        Q_PROPERTY(bool inProgress READ getInProgress NOTIFY inProgressChanged)
        Q_PROPERTY(bool isError READ getHasErrors NOTIFY hasErrorsChanged)
        Q_PROPERTY(int itemsCount READ getItemsCount NOTIFY itemsCountChanged)
    public:
        ArtworksUploader(Common::ISystemEnvironment &environment,
                         Models::UploadInfoRepository &uploadInfoRepository,
                         SettingsModel &settingsModel,
                         QObject *parent=nullptr);

    public:
        // used to test UI of artwork upload
        // virtual bool getInProgress() const { return true; }
        void setFtpCoordinator(const std::shared_ptr<libxpks::net::FtpCoordinator> &ftpCoordinator);

    signals:
        void inProgressChanged();
        void hasErrorsChanged();
        void percentChanged();
        void itemsCountChanged();
        void startedProcessing();
        void finishedProcessing();
        void requestCloseWindow();
        void credentialsChecked(bool result, const QString &url);

    public:
        int getUIPercent() const { return ((PERCENT_EPSILON < m_Percent) && (m_Percent < 1.0)) ? 1 : (int)m_Percent; }
        bool getInProgress() const { return m_IsInProgress; }
        bool getHasErrors() const { return m_HasErrors; }
        int getItemsCount() const { return (int)m_ArtworksSnapshot.size(); }

    public:
        void setPercent(double value);
        void setInProgress(bool value);
        void setHasErrors(bool value);

    public slots:
        void onUploadStarted();
        void allFinished(bool anyError);
        void credentialsTestingFinished();

    private slots:
        void uploaderPercentChanged(double percent);

    public:
        void setArtworks(Artworks::ArtworksSnapshot &snapshot);
        virtual void handleMessage(UnavailableFilesMessage const &message) override;

    public:
        Q_INVOKABLE void uploadArtworks();
        Q_INVOKABLE void checkCredentials(const QString &host, const QString &username,
                                          const QString &password, bool disablePassiveMode, bool disableEPSV);
        Q_INVOKABLE bool needCreateArchives() const;

        Q_INVOKABLE QString getFtpName(const QString &stockAddress) const;
        Q_INVOKABLE QObject *getUploadWatcherObject();
        Q_INVOKABLE QObject *getUploadInfosObject();

        Q_INVOKABLE void resetModel();
        Q_INVOKABLE void clearModel();
        Q_INVOKABLE void resetProgress();
        Q_INVOKABLE void cancelOperation();

    protected:
        void resetArtworks();

#ifdef CORE_TESTS
    public:
#else
    protected:
#endif
        const Artworks::ArtworksSnapshot &getArtworksSnapshot() const { return m_ArtworksSnapshot; }

#ifdef INTEGRATION_TESTS
    public:
        Connectivity::UploadWatcher &accessWatcher() { return m_UploadWatcher; }
#endif

    private:
        void doUploadArtworks(const Artworks::ArtworksSnapshot &snapshot);

    protected:
        bool removeUnavailableItems();

    private:
        Common::ISystemEnvironment &m_Environment;
        Artworks::ArtworksSnapshot m_ArtworksSnapshot;
        Connectivity::UploadWatcher m_UploadWatcher;
        std::shared_ptr<Connectivity::IFtpCoordinator> m_FtpCoordinator;
        QFutureWatcher<Connectivity::ContextValidationResult> m_TestingCredentialWatcher;
        UploadInfoRepository &m_UploadInfos;
        SettingsModel &m_SettingsModel;
        double m_Percent;
        volatile bool m_IsInProgress;
        volatile bool m_HasErrors;
    };
}

#endif // ARTWORKSUPLOADER_H
