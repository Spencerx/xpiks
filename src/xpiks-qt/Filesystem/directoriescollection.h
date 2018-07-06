/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DIRECTORIESCOLLECTION_H
#define DIRECTORIESCOLLECTION_H

#include <memory>
#include <QList>
#include <QUrl>
#include "ifilescollection.h"
#include "filescollection.h"

namespace Filesystem {
    class DirectoriesCollection: public IFilesCollection
    {
    public:
        DirectoriesCollection(const QStringList &directories);
        DirectoriesCollection(const QList<QUrl> &urls);

    private:
        void extractFiles(const QStringList &directories);

        // IFilesCollection interface
    public:
        virtual const std::vector<ArtworkFile> &getFiles() { return m_Files->getFiles(); }

    private:
        std::shared_ptr<FilesCollection> m_Files;
    };
}

#endif // DIRECTORIESCOLLECTION_H
