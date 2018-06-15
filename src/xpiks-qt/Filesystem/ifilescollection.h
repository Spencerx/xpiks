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

#include <QString>
#include <vector>

namespace Filesystem {
    enum struct ArtworkFileType {
        Image,
        Video,
        Vector
    };

    struct ArtworkFile {
        ArtworkFile(const QString &path, ArtworkFileType type):
            m_Path(path),
            m_Type(type)
        { }

        QString m_Path;
        ArtworkFileType m_Type;
    };

    class IFilesCollection {
    public:
        virtual ~IFilesCollection() {}

    public:
        virtual const std::vector<ArtworkFile> &getFiles() = 0;
    };
}

#endif // IFILESCOLLECTION_H
