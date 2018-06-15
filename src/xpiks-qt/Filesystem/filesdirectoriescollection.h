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
        virtual const QStringList &getImages() override { return m_Images; }
        virtual const QStringList &getVectors() override { return m_Vectors; }
        virtual const QStringList &getVideos() override { return m_Videos; }

    private:
        QStringList m_Images;
        QStringList m_Videos;
        QStringList m_Vectors;
    };
}

#endif // FILESDIRECTORIESCOLLECTION_H
