/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ISYSTEMENVIRONMENT_H
#define ISYSTEMENVIRONMENT_H

#include <QString>

namespace Common {
    class ISystemEnvironment {
    public:
        virtual ~ISystemEnvironment() {}
        virtual QString root() const = 0;
        virtual QString fileInDir(const QString &filename, const QString &dirname) = 0;
        virtual QString filepath(const QString &name) = 0;
        virtual QString dirpath(const QString &name) = 0;
        virtual void ensureDirExists(const QString &name) = 0;
    };
}

#endif // ISYSTEMENVIRONMENT_H
