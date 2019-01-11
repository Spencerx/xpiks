/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "remoteconfig.h"

#include <QJsonParseError>
#include <QtDebug>

#include "Common/logging.h"

namespace Helpers {
    RemoteConfig::RemoteConfig(const QString &configUrl, QObject *parent):
        QObject(parent),
        m_ConfigUrl(configUrl)
    {
    }

    RemoteConfig::~RemoteConfig() {
    }

    void RemoteConfig::setRemoteResponse(const QByteArray &responseData) {
        QJsonParseError error;
        LOG_VERBOSE_OR_DEBUG << responseData;

        m_Config = QJsonDocument::fromJson(responseData, &error);

        if (error.error == QJsonParseError::NoError) {
            emit configArrived();
        } else {
            LOG_VERBOSE << m_ConfigUrl;
            LOG_WARNING << "Failed to parse remote json" << error.errorString();
        }
    }
}
