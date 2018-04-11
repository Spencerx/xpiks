/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ICONNECTIVITYREQUEST_H
#define ICONNECTIVITYREQUEST_H

#include <QString>
#include <QByteArray>
#include "../Common/flags.h"

namespace Connectivity {
    enum RequestFlags {
        None = 1 << 0,
        NoCache = 1 << 1
    };

    class IConnectivityRequest {
    public:
        virtual ~IConnectivityRequest() {}

    public:
        virtual QString getResourceURL() const = 0;
        virtual QStringList getRawHeaders() const = 0;
        virtual Common::flag_t getFlags() const = 0;
        virtual void setResponse(const QByteArray &responseData) = 0;
    };
}

#endif // ICONNECTIVITYREQUEST_H
