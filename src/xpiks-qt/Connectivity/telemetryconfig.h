/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TELEMETRYCONFIG_H
#define TELEMETRYCONFIG_H

#include <QObject>
#include <QString>

#include "Models/Connectivity/abstractconfigupdatermodel.h"

namespace Common {
    class ISystemEnvironment;
}

namespace Connectivity {
    class TelemetryConfig: public Models::AbstractConfigUpdaterModel
    {
        Q_OBJECT
    public:
        TelemetryConfig(Common::ISystemEnvironment &environment,
                        QObject *parent=nullptr);

    public:
        QString getEndpoint();

    signals:
        void configUpdated();

        // AbstractConfigUpdaterModel interface
    protected:
        virtual bool processLocalConfig(const QJsonDocument &) override { /*ignore local config*/ return false; }
        virtual void processMergedConfig(const QJsonDocument &document) override;

    private:
        bool parseConfig(const QJsonDocument &document);

        // CompareValuesJson interface
    public:
        virtual int operator ()(const QJsonObject &val1, const QJsonObject &val2) override;

    private:
        QString m_Endpoint;
    };
}

#endif // TELEMETRYCONFIG_H
