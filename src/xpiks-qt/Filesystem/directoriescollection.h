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

#include <initializer_list>
#include <memory>
#include <vector>

#include <QList>
#include <QStringList>

#include "Filesystem/filescollection.h"
#include "Filesystem/ifilescollection.h"

class QString;
class QUrl;

namespace Filesystem {
    class DirectoriesCollection: public IFilesCollection
    {
    public:
        DirectoriesCollection(const QStringList &directories);
        DirectoriesCollection(const QList<QUrl> &urls);
        DirectoriesCollection(std::initializer_list<QString> directories);

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
