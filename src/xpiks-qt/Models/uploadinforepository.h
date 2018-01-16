/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UPLOADINFOREPOSITORY_H
#define UPLOADINFOREPOSITORY_H

#include <QAbstractListModel>
#include <QDataStream>
#include <QByteArray>
#include <QHash>
#include <QList>
#include <vector>
#include <memory>
#include "../Common/baseentity.h"
#include "../Common/delayedactionentity.h"
#include "../Helpers/localconfig.h"

namespace Models {
    class UploadInfo;

    class UploadInfoRepository:
            public QAbstractListModel,
            public Common::BaseEntity,
            public Common::DelayedActionEntity
    {
        Q_OBJECT
        Q_PROPERTY(int infosCount READ getInfosCount NOTIFY infosCountChanged)

    public:
        UploadInfoRepository(QObject *parent = 0);
        virtual ~UploadInfoRepository();

        void initFromString(const QString &savedString);

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
        void initializeConfig();

    private:

    public:
        Q_INVOKABLE void removeItem(int row);
        Q_INVOKABLE void addItem();
        Q_INVOKABLE int getSelectedInfosCount() const;
        Q_INVOKABLE QString getAgenciesWithMissingDetails();
        Q_INVOKABLE void updateProperties(int itemIndex);

        Q_INVOKABLE void initializeAccounts(bool mpIsCorrectOrEmpty);
        Q_INVOKABLE void finalizeAccounts();

        Q_INVOKABLE bool isMasterPasswordCorrectOrEmpty() const { return !m_EmptyPasswordsMode; }

    public:
        void setEmptyPasswordsMode(bool mode) { m_EmptyPasswordsMode = mode; }
        void backupAndDropRealPasswords();
        void restoreRealPasswords();

    public:
        void updatePercentages();
        void resetPercents();

#ifdef INTEGRATION_TESTS
    public:
        std::shared_ptr<UploadInfo> appendItem() { addItem(); return m_UploadInfos.back(); }
#endif

    public:
        const std::vector<std::shared_ptr<UploadInfo > > &getUploadInfos() const { return m_UploadInfos; }
        std::vector<std::shared_ptr<UploadInfo> > retrieveSelectedUploadInfos() const;

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

    private slots:
        void onBackupRequired();

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
        std::vector<std::shared_ptr<UploadInfo> > m_UploadInfos;
        Helpers::LocalConfig m_LocalConfig;
        // when MP is cancelled before Upload dialog
        // all passwords should be empty
        bool m_EmptyPasswordsMode;
    };
}

#endif // UPLOADINFOREPOSITORY_H
