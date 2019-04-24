/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UPLOADINFOREPOSITORY_H
#define UPLOADINFOREPOSITORY_H

#include <memory>
#include <vector>

#include <QAbstractListModel>
#include <QHash>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>

#include "Common/delayedactionentity.h"
#include "Helpers/localconfig.h"
#include "Microstocks/stocksftplistmodel.h"
#include "Services/AutoComplete/stringsautocompletemodel.h"

class QByteArray;
class QModelIndex;
class QTimerEvent;

namespace Common {
    class ISystemEnvironment;
}

namespace Connectivity {
    class IRequestsService;
}

namespace Helpers {
    class AsyncCoordinator;
}

namespace Encryption {
    class SecretsManager;
}

namespace Models {
    class UploadInfo;

    class UploadInfoRepository:
            public QAbstractListModel,
            public Common::DelayedActionEntity
    {
        Q_OBJECT
        Q_PROPERTY(int infosCount READ getInfosCount NOTIFY infosCountChanged)

    public:
        UploadInfoRepository(Common::ISystemEnvironment &environment,
                             Encryption::SecretsManager &secretsManager,
                             QObject *parent = nullptr);
        virtual ~UploadInfoRepository() override;

    public:
        enum UploadInfoRepository_Roles {
            TitleRole = Qt::UserRole + 1,
            EditTitleRole,
            HostRole,
            EditHostRole,
            UsernameRole,
            EditUsernameRole,
            PasswordRole,
            EditPasswordRole,
            IsSelectedRole,
            EditIsSelectedRole,
            ZipBeforeUploadRole,
            EditZipBeforeUploadRole,
            EditUploadDirectoryRole,
            PercentRole,
            /*DEPRECATED*/ FtpPassiveModeRole,
            /*DEPRECATED*/ EditFtpPassiveModeRole,
            DisableFtpPassiveModeRole,
            EditDisableFtpPassiveModeRole,
            DisableEPSVRole,
            EditDisableEPSVRole,
            VectorFirstRole,
            EditVectorFirstRole
        };

    public:
        int getInfosCount() const { return (int)m_UploadInfos.size(); }

    public:
        void initFromString(const QString &savedString);
        void initializeConfig();        
        void initializeStocksList(Helpers::AsyncCoordinator &initCoordinator,
                                  Connectivity::IRequestsService &requestsService);

    public:
        Q_INVOKABLE void removeItem(int row);
        Q_INVOKABLE void addItem();
        Q_INVOKABLE int getSelectedInfosCount() const;
        Q_INVOKABLE QString getAgenciesWithMissingDetails();
        Q_INVOKABLE void updateProperties(int itemIndex);
        Q_INVOKABLE void setCurrentIndex(int index);
        Q_INVOKABLE void initializeAccounts(bool mpIsCorrectOrEmpty);
        Q_INVOKABLE void finalizeAccounts();
        Q_INVOKABLE bool isMasterPasswordCorrectOrEmpty() const { return !m_EmptyPasswordsMode; }
        Q_INVOKABLE void validateCurrentItem();
        Q_INVOKABLE QObject *getStocksCompletionObject();
#ifdef UI_TESTS
        Q_INVOKABLE void clear() { clearHosts(); }
#endif

    public:
        void setEmptyPasswordsMode(bool mode) { m_EmptyPasswordsMode = mode; }
        void backupAndDropRealPasswords();
        void restoreRealPasswords();

    public:
        void updatePercentages();
        void resetPercents();

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS) || defined(CORE_TESTS)
    public:
        AutoComplete::StringsAutoCompleteModel &getStocksCompletionSource() { return m_StocksCompletionSource; }
        std::shared_ptr<UploadInfo> appendItem() { addItem(); return m_UploadInfos.back(); }
        std::vector<std::shared_ptr<UploadInfo>> &accessUploadInfos() { return m_UploadInfos; }
        Microstocks::StocksFtpListModel &accessStocksList() { return m_StocksFtpList; }
        void clearHosts();
#endif

    public:
        std::vector<std::shared_ptr<UploadInfo>> retrieveSelectedUploadInfos() const;

    public:
        bool isZippingRequired() const;
        std::shared_ptr<UploadInfo> tryFindItemByHost(const QString &stockAddress);

    public:
        virtual int rowCount(const QModelIndex &parent=QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) override;

    signals:
        void infosCountChanged();
        void backupRequired();

    public slots:
        void onBeforeMasterPasswordChanged(const QString &oldMasterPassword, const QString &newMasterPassword);
        void onAfterMasterPasswordReset();
        void onCompletionSelected(int completionID);

    private slots:
        void onBackupRequired();
        void onStocksListUpdated();

    protected:
        bool saveUploadInfos();
        virtual QHash<int, QByteArray> roleNames() const override;
        void removeInnerItem(int row);

        // DelayedActionEntity implementation
    protected:
        virtual void doKillTimer(int timerId) override { this->killTimer(timerId); }
        virtual int doStartTimer(int interval, Qt::TimerType timerType) override { return this->startTimer(interval, timerType); }
        virtual void doOnTimer() override { emit backupRequired(); }
        virtual void timerEvent(QTimerEvent *event) override { onQtTimer(event); }
        virtual void callBaseTimer(QTimerEvent *event) override { QObject::timerEvent(event); }

    private:
        Common::ISystemEnvironment &m_Environment;
        std::vector<std::shared_ptr<UploadInfo>> m_UploadInfos;
        Helpers::LocalConfig m_LocalConfig;
        AutoComplete::StringsAutoCompleteModel m_StocksCompletionSource;
        Microstocks::StocksFtpListModel m_StocksFtpList;
        Encryption::SecretsManager &m_SecretsManager;
        int m_CurrentIndex;
        // when MP is cancelled before Upload dialog
        // all passwords should be empty
        bool m_EmptyPasswordsMode;
    };
}

#endif // UPLOADINFOREPOSITORY_H
