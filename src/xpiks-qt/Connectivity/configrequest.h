/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONNECTIVITYREQUEST_H
#define CONNECTIVITYREQUEST_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include "iconnectivityrequest.h"

namespace Helpers {
    class RemoteConfig;
}

namespace Connectivity {
    class ConfigRequest:
            public QObject,
            public IConnectivityRequest,
            public IConnectivityResponse {
        Q_OBJECT

    public:
        ConfigRequest(Helpers::RemoteConfig *config, const QString &url, bool noCache = false):
            QObject(),
            m_RemoteConfig(config),
            m_Url(url),
            m_Flags(0)
        {
            Q_ASSERT(config != nullptr);
            m_Flags = (Common::flag_t)(noCache ? Connectivity::NoCache : Connectivity::None);
        }

        // IConnectivityRequest interface
    public:
        virtual QString getResourceURL() override { return m_Url; }
        virtual QStringList getRawHeaders() override { return QStringList(); }
        virtual Common::flag_t getFlags() override { return m_Flags; }
        virtual IConnectivityResponse *getResponse() override { return this; }

        // IConnectivityResponse interface
    public:
        virtual void setResult(bool result, const QByteArray &body) override;

    private:
        Helpers::RemoteConfig *m_RemoteConfig;
        QString m_Url;
        Common::flag_t m_Flags;
    };
}

#endif // CONNECTIVITYREQUEST_H
