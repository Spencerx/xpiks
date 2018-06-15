/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef FILESDIRECTORIESCOLLECTION_H
#define FILESDIRECTORIESCOLLECTION_H

#include "ifilescollection.h"
#include <QList>
#include <QUrl>

namespace Filesystem {
    class FilesDirectoriesCollection: public IFilesCollection
    {
    public:
        FilesDirectoriesCollection(const QList<QUrl> &urls);

    private:
        void sortFilesDirectories(const QList<QUrl> &urls);

        // IFilesCollection interface
    public:
        virtual const std::vector<ArtworkFile> &getFiles() { return m_Files; }

    private:
        std::vector<ArtworkFile> m_Files;
    };
}

#endif // FILESDIRECTORIESCOLLECTION_H
