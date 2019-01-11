/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ftphelpers.h"

#include <QChar>
#include <QLatin1String>
#include <QString>

namespace Connectivity {
    QString sanitizeHost(const QString &inputHost) {
        QString host = inputHost;

        const QChar slash('/');

        if (!host.endsWith(slash)) {
            host.append(slash);
        }

        if (!host.startsWith(QLatin1String("ftp.")) &&
            !host.startsWith(QLatin1String("ftp://"))) {
            host = QLatin1String("ftp://") + host;
        }

        return host;
    }
}
