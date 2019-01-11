/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CURLHELPERS_H
#define CURLHELPERS_H

#include <memory>

#include <QString>

namespace Models {
    class ProxySettings;
}

namespace libxpks {
    namespace net {
        class UploadContext;
    }
}

namespace Connectivity {
    void fillCurlOptions(void *curlHandle, const std::shared_ptr<libxpks::net::UploadContext> &context, const QString &remoteUrl);
    void fillProxySettings(void *curlHandle, const Models::ProxySettings *proxySettings);
}

#endif // CURLHELPERS_H
