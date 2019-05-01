/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SWITCHERCONFIG_H
#define SWITCHERCONFIG_H

#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QReadWriteLock>
#include <QString>

#include "Models/Connectivity/abstractconfigupdatermodel.h"

namespace Common {
    class ISystemEnvironment;
}

namespace Connectivity {
    class SwitcherConfig: public Models::AbstractConfigUpdaterModel
    {
        Q_OBJECT
    public:
        struct SwitchValue {
            bool m_IsOn;
            int m_Threshold;
        };

    public:
        SwitcherConfig(Common::ISystemEnvironment &environment,
                       QObject *parent=nullptr);

    public:
        bool isSwitchOn(int switchKey, int minThreshold);

    signals:
        void switchesUpdated();

        // AbstractConfigUpdaterModel interface
    protected:
        virtual bool processLocalConfig(const QJsonDocument &document) override;
        virtual void processMergedConfig(const QJsonDocument &document) override;

        // CompareValuesJson interface
    public:
        virtual int operator ()(const QJsonObject &val1, const QJsonObject &val2) override;

    private:
        void parseSwitches(const QJsonObject &object);

    public:
        enum Switches_Keys {
            DonateCampaign1=0,
            DonateCampaign1Stage2,
            ProgressiveSuggestionPreviews,
            DirectMetadataExport,
            GoodQualityVideoPreviews,
            MetadataAutoImport,
            GettySuggestionEnabled,
            UpdateEnabled,
            KeywordsDragDropEnabled,
            TelemetryEnabled
        };

    private:
        QReadWriteLock m_RwLock;
        QHash<int, SwitchValue> m_SwitchesHash;
    };
}

#endif // SWITCHERCONFIG_H
