/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CSVEXPORTMODEL_H
#define CSVEXPORTMODEL_H

#include <QAbstractListModel>
#include <vector>
#include <memory>
#include "csvexportproperties.h"
#include "artworkssnapshot.h"
#include "csvexportplansmodel.h"
#include "../Common/baseentity.h"
#include "../Common/delayedactionentity.h"

class QTimerEvent;

namespace MetadataIO {
    class CsvExportColumnsModel: public QAbstractListModel
    {
        Q_OBJECT
    public:
        CsvExportColumnsModel();

    public:
        int getCurrentIndex() const { return m_CurrentIndex; }

    public:
        void setupModel(int row, const std::shared_ptr<CsvExportPlan> &plan);
        void clearModel();

    private:
        enum CsvExportColumnsModel_Roles {
            ColumnNameRole = Qt::DisplayRole + 1,
            PropertyTypeRole,
            EditColumnNameRole,
            EditPropertyRole
        };

    public:
        Q_INVOKABLE void addColumn();
        Q_INVOKABLE void addColumnAbove(int row);
        Q_INVOKABLE void removeColumn(int row);
        Q_INVOKABLE void moveColumnUp(int row);
        Q_INVOKABLE void moveColumnDown(int row);
        Q_INVOKABLE QStringList getPropertiesList();

        // QAbstractItemModel interface
    public:
        virtual int rowCount(const QModelIndex &parent) const override;
        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual QHash<int, QByteArray> roleNames() const override;

    private:
        int getColumnsCount() const;

    private:
        std::shared_ptr<CsvExportPlan> m_ExportPlan;
        int m_CurrentIndex;
    };

    class CsvExportModel:
            public QAbstractListModel,
            public Common::BaseEntity,
            public Common::DelayedActionEntity
    {
        Q_OBJECT
        Q_PROPERTY(bool isExporting READ getIsExporting WRITE setIsExporting NOTIFY isExportingChanged)
        Q_PROPERTY(int artworksCount READ getArtworksCount NOTIFY artworksCountChanged)
    public:
        CsvExportModel();

    public:
        const std::vector<std::shared_ptr<CsvExportPlan> > &getExportPlans() const { return m_ExportPlans; }
        bool getIsExporting() const { return m_IsExporting; }
        int getArtworksCount() const { return (int)m_ArtworksToExport.size(); }

    public:
        void setIsExporting(bool value);

    public:
        virtual void setCommandManager(Commands::CommandManager *commandManager) override;

    public:
        void setupModel(ArtworksSnapshot::Container &rawSnapshot);
        void initializeExportPlans(Helpers::AsyncCoordinator *initCoordinator);

#ifdef INTEGRATION_TESTS
    public:
        void disableRemoteConfigs() { m_ExportPlansModel.setOnlyLocal(); }
#endif

    private:
        enum CsvExportModel_Roles {
            PlanNameRole = Qt::DisplayRole + 1,
            IsSelectedRole,
            IsSystemPlanRole,
            EditIsSelectedRole,
            EditPlanNameRole
        };

        // QAbstractItemModel interface
    public:
        virtual int rowCount(const QModelIndex &parent) const override;
        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QHash<int, QByteArray> roleNames() const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    public:
        Q_INVOKABLE void startExport();
        Q_INVOKABLE void clearModel();
        Q_INVOKABLE void removePlanAt(int row);
        Q_INVOKABLE void addNewPlan();
        Q_INVOKABLE QObject *getColumnsModel();
        Q_INVOKABLE void setCurrentItem(int row);
        Q_INVOKABLE void requestSave();
        Q_INVOKABLE int getSelectedPlansCount() { return retrieveSelectedPlansCount(); }
        Q_INVOKABLE void setOutputDirectory(const QUrl &url);

    private:
        void saveExportPlans();
        int retrieveSelectedPlansCount();

#ifdef INTEGRATION_TESTS
    public:
        std::vector<std::shared_ptr<CsvExportPlan> > &accessExportPlans() { return m_ExportPlans; }
        void clearPlans() { m_ExportPlans.clear(); }
#endif

    signals:
        void isExportingChanged();
        void artworksCountChanged();
        void backupRequired();
        void exportFinished();

    private slots:
        void onWorkerFinished();
        void onPlansUpdated();
        void onBackupRequired();

        // DelayedActionEntity implementation
    protected:
        virtual void doKillTimer(int timerId) override { this->killTimer(timerId); }
        virtual int doStartTimer(int interval, Qt::TimerType timerType) override { return this->startTimer(interval, timerType); }
        virtual void doOnTimer() override { emit backupRequired(); }
        virtual void timerEvent(QTimerEvent *event) override { onQtTimer(event); }
        virtual void callBaseTimer(QTimerEvent *event) override { QAbstractListModel::timerEvent(event); }

    private:
        CsvExportColumnsModel m_CurrentColumnsModel;
        CsvExportPlansModel m_ExportPlansModel;
        std::vector<std::shared_ptr<CsvExportPlan> > m_ExportPlans;
        ArtworksSnapshot m_ArtworksToExport;
        QString m_ExportDirectory;
        int m_SaveTimerId;
        int m_SaveRestartsCount;
        bool m_IsExporting;
    };
}

#endif // CSVEXPORTMODEL_H
