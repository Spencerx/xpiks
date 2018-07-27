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
#include <Common/flags.h>

namespace Filesystem {
    enum struct ArtworkFileType: uint8_t {
        Image,
        Video,
        Vector
    };

    struct ArtworkFile {
        ArtworkFile(const QString &path, ArtworkFileType type, bool isPartOfFullDirectory=false):
            m_Path(path),
            m_Type(type),
            m_IsPartOfFullDirectory(isPartOfFullDirectory)
        { }

        bool isPartOfFullDirectory() const { return m_IsPartOfFullDirectory; }

        QString m_Path;
        ArtworkFileType m_Type;
        bool m_IsPartOfFullDirectory;
    };

    class IFilesCollection {
    public:
        virtual ~IFilesCollection() {}
        virtual std::vector<ArtworkFile> const &getFiles() = 0;
    };
}

#endif // IFILESCOLLECTION_H
