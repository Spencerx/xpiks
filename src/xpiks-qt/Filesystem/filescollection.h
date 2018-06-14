/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef FILESCOLLECTION_H
#define FILESCOLLECTION_H

#include "ifilescollection.h"
#include <QList>
#include <QUrl>

namespace Filesystem {
    class FilesCollection: public IFilesCollection
    {
    public:
        FilesCollection(const QList<QUrl> &urls);
        FilesCollection(const QStringList &files);

    private:
        void sortRawFiles(const QStringList &files);

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

#endif // FILESCOLLECTION_H
