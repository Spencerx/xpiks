/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ISECRETSSTORAGE_H
#define ISECRETSSTORAGE_H

#include "secretpair.h"

namespace Encryption {
    class ISecretsStorage {
    public:
        virtual ~ISecretsStorage() { }
        virtual bool tryFindPair(int key, SecretPair &secretPair) = 0;
        virtual bool insertPair(int key, const SecretPair &SecretPair) = 0;
    };
}

#endif // ISECRETSSTORAGE_H
