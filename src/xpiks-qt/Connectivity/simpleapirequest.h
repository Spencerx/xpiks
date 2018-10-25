/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SIMPLEAPIREQUEST_H
#define SIMPLEAPIREQUEST_H

#include <memory>

#include "Connectivity/iconnectivityrequest.h"

namespace Connectivity {
    class SimpleAPIRequest: public IConnectivityRequest
    {
    public:
        SimpleAPIRequest(const QString &url, const QStringList &headers, const std::shared_ptr<IConnectivityResponse> &response):
            m_URL(url),
            m_Headers(headers),
            m_Flags(IConnectivityRequest::None),
            m_Response(response)
        { }

        // IConnectivityRequest interface
    public:
        virtual QString getResourceURL() override { return m_URL; }
        virtual QStringList getRawHeaders() override { return m_Headers; }
        virtual Common::flag_t getFlags() override { return m_Flags; }
        virtual IConnectivityResponse *getResponse() override { return m_Response.get(); }

    private:
        QString m_URL;
        QStringList m_Headers;
        Common::flag_t m_Flags;
        std::shared_ptr<IConnectivityResponse> m_Response;
    };
}

#endif // SIMPLEAPIREQUEST_H
