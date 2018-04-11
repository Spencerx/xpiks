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
    class ConnectivityRequest: public QObject, public IConnectivityRequest {
        Q_OBJECT

    public:
        ConnectivityRequest(Helpers::RemoteConfig *config, const QString &url, bool noCache = false):
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
        virtual QString getResourceURL() const override { return m_Url; }
        virtual QStringList getRawHeaders() const override { return QStringList(); }
        virtual Common::flag_t getFlags() const override { return m_Flags; }
        virtual void setResponse(const QByteArray &responseData) override;

    public:
        const QString &getURL() const { return m_Url; }

    signals:
        void responseReceived(bool success);

    private:
        Helpers::RemoteConfig *m_RemoteConfig;
        QString m_Url;
        Common::flag_t m_Flags;
    };
}

#endif // CONNECTIVITYREQUEST_H
