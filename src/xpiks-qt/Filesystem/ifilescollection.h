/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IFILESCOLLECTION_H
#define IFILESCOLLECTION_H

#include <QStringList>

namespace Filesystem {
    class IFilesCollection {
    public:
        virtual ~IFilesCollection() {}

    public:
        virtual const QStringList &getImages() = 0;
        virtual const QStringList &getVectors() = 0;
        virtual const QStringList &getVideos() = 0;
    };
}

#endif // IFILESCOLLECTION_H
