/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef HASHHELPERS_H
#define HASHHELPERS_H

#include <cstddef>
#include <system_error>

#include <QHash>
#include <QString>

namespace std {
    template<> struct hash<QString> {
        size_t operator() (QString const &s) const {
            return qHash(s);
        }
    };
}

#endif // HASHHELPERS_H
