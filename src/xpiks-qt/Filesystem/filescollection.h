/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef FILESCOLLECTION_H
#define FILESCOLLECTION_H

#include <initializer_list>
#include <vector>

#include <QList>
#include <QStringList>

#include "Filesystem/ifilescollection.h"

class QUrl;

namespace Filesystem {
    class FilesCollection: public IFilesCollection
    {
    public:
        FilesCollection(QList<QUrl> const &urls, bool fullDirectory=false);
        FilesCollection(QStringList const &files, bool fullDirectory=false);
        FilesCollection(std::initializer_list<QStringList> filesList);

    private:
        void sortRawFiles(QStringList const &files, bool fullDirectory);

        // IFilesCollection interface
    public:
        virtual const std::vector<ArtworkFile> &getFiles() { return m_Files; }

    private:
        std::vector<ArtworkFile> m_Files;
    };
}

#endif // FILESCOLLECTION_H
