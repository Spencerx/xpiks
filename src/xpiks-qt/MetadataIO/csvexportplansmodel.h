/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CSVEXPORTPLANSMODEL_H
#define CSVEXPORTPLANSMODEL_H

#include <QJsonObject>
#include <vector>
#include <memory>
#include "csvexportproperties.h"
#include <Models/Connectivity/abstractconfigupdatermodel.h>
#include <Helpers/localconfig.h>
#include "../Common/isystemenvironment.h"

namespace Helpers {
    class AsyncCoordinator;
}

namespace MetadataIO {
    class CsvExportPlansModel:
            public Models::AbstractConfigUpdaterModel
    {
        Q_OBJECT
    public:
        CsvExportPlansModel(Common::ISystemEnvironment &environment,
                            QObject *parent = nullptr);

    public:
        std::vector<std::shared_ptr<CsvExportPlan> > &getExportPlans() { return m_ExportPlans; }

    public:
        void initializeConfigs(Helpers::AsyncCoordinator &initCoordinator,
                               Connectivity::RequestsService &requestsService);
        void sync(const std::vector<std::shared_ptr<CsvExportPlan> > &exportPlans);

        // AbstractConfigUpdaterModel interface
    protected:
        virtual bool processLocalConfig(const QJsonDocument &document) override;
        virtual void processRemoteConfig(const QJsonDocument &remoteDocument, bool overwriteLocal) override;
        virtual void processMergedConfig(const QJsonDocument &document) override;

        // CompareValuesJson interface
    public:
        virtual int operator ()(const QJsonObject &val1, const QJsonObject &val2) override;

    signals:
        void plansUpdated();

    private:
        void deserializeExportPlans(const QJsonObject &object);

    private:
        Common::ISystemEnvironment &m_Environment;
        std::vector<std::shared_ptr<CsvExportPlan> > m_ExportPlans;
    };
}

#endif // CSVEXPORTPLANSMODEL_H
