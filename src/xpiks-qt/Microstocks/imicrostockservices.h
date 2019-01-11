/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IMICROSTOCKSERVICES_H
#define IMICROSTOCKSERVICES_H

#include <memory>

#include "Microstocks/microstockenums.h"

namespace Microstocks {
    class IMicrostockService;

    class IMicrostockServices {
    public:
        virtual ~IMicrostockServices() {}

        virtual std::shared_ptr<IMicrostockService> getService(MicrostockType type) = 0;
    };
}

#endif // IMICROSTOCKSERVICES_H
