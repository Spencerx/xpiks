/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef STOCKFTPOPTIONS_H
#define STOCKFTPOPTIONS_H

#include <QString>

namespace Microstocks {
#define DEFAULT_STOCK_ID 0

    struct StockFtpOptions {
        int m_ID = DEFAULT_STOCK_ID;
        QString m_Title;
        QString m_FtpAddress;
        QString m_ImagesDir;
        QString m_VectorsDir;
        QString m_VideosDir;
        bool m_ZipVector = false;
    };
}

#endif // STOCKFTPOPTIONS_H
