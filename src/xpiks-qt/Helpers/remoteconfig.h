/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef REMOTECONFIG_H
#define REMOTECONFIG_H

#include <QByteArray>
#include <QJsonDocument>
#include <QObject>
#include <QString>

namespace Connectivity {
    class ConfigRequest;
}

#if defined(CORE_TESTS)
namespace Mocks {
    class RequestsServiceMock;
}
#endif

namespace Helpers {
    class RemoteConfig : public QObject {
        Q_OBJECT
    public:
        RemoteConfig(const QString &configUrl, QObject *parent=0);
        virtual ~RemoteConfig();

    public:
        const QJsonDocument& getConfig() const { return m_Config; }
        const QString &getUrl() const { return m_ConfigUrl; }

    private:
        friend class Connectivity::ConfigRequest;
#if defined(CORE_TESTS)
        friend class Mocks::RequestsServiceMock;
#endif
        void setRemoteResponse(const QByteArray &responseData);

    signals:
        void configArrived();

    private:
        QString m_ConfigUrl;
        QJsonDocument m_Config;
    };
}

#endif // REMOTECONFIG_H
