/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "telemetryconfig.h"

#include <QJsonObject>
#include <QJsonValue>

#include "Common/isystemenvironment.h"
#include "Common/logging.h"
#include "Connectivity/apimanager.h"
#include "Encryption/aes-qt.h"

#define LOCAL_TELEMETRY_CONFIG_FILE QLatin1String("telemetry.json")
#define SETTINGS_KEY QLatin1String("settings")
#define TELEMETRY_ENDPOINT QLatin1String("endpoint")

#define OVERWRITE_TELEMETRY_CONFIG true

namespace Connectivity {
    TelemetryConfig::TelemetryConfig(Common::ISystemEnvironment &environment, QObject *parent):
        Models::AbstractConfigUpdaterModel(
            environment.path({LOCAL_TELEMETRY_CONFIG_FILE}),
            ApiManager::getInstance().getTelemetryConfigAddr(),
            OVERWRITE_TELEMETRY_CONFIG,
            environment.getIsInMemoryOnly())
    {
        m_Endpoint = QLatin1String(
                    /*ignorestyle*/"cc39a47f60e1ed812e2403b33678dd1c529f1cc43f66494998ec478a4d13496269a3dfa01f882941766dba246c76b12b2a0308e20afd84371c41cf513260f8eb8b71f8c472cafb1abf712c071938ec0791bbf769ab9625c3b64827f511fa3fbb");
    }

    QString TelemetryConfig::getEndpoint() {
        QString endpoint = Encryption::decodeText(m_Endpoint, "reporting");
        return endpoint;
    }

    void TelemetryConfig::processMergedConfig(const QJsonDocument &document) {
        LOG_DEBUG << "#";
        bool anyError = false;

        do {
            if (!document.isObject()) {
                LOG_WARNING << "Json document is not an object";
                anyError = true;
                break;
            }

            QJsonObject rootObject = document.object();
            if (!rootObject.contains(SETTINGS_KEY)) {
                LOG_WARNING << "There's no settings key in json";
                anyError = true;
                break;
            }

            QJsonValue settingsValue = rootObject[SETTINGS_KEY];
            if (!settingsValue.isObject()) {
                LOG_WARNING << "Settings object is not an object";
                anyError = true;
                break;
            }

            QJsonObject settingsObject = settingsValue.toObject();

            {
                QJsonValue endpoint = settingsObject[TELEMETRY_ENDPOINT];
                if (!endpoint.isString()) {
                    LOG_WARNING << "TELEMETRY_ENDPOINT value is not string";
                    anyError = true;
                    break;
                }

                m_Endpoint = endpoint.toString();
            }
        } while(false);

        LOG_VERBOSE_OR_DEBUG << "Any error:" << anyError;
        if (!anyError) {
            emit configUpdated();
        }
    }

    int TelemetryConfig::operator ()(const QJsonObject &val1, const QJsonObject &val2) {
        Q_UNUSED(val1);
        Q_UNUSED(val2);
        return 0;
    }
}
