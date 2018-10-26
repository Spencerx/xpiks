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

#include "Common/types.h"

class QStringList;

namespace Connectivity {
    class IConnectivityResponse;

    class IConnectivityRequest {
    public:
        enum RequestFlags {
            None = 1 << 0,
            NoCache = 1 << 1
        };
    public:
        virtual ~IConnectivityRequest() {}

    public:
        virtual QString getResourceURL() = 0;
        virtual QStringList getRawHeaders() = 0;
        virtual Common::flag_t getFlags() = 0;
        virtual IConnectivityResponse *getResponse() = 0;
    };
}

#endif // ICONNECTIVITYREQUEST_H
