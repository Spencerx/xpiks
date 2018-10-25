/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "directoriescollection.h"

#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QString>
#include <QUrl>
#include <QtDebug>

#include "Common/logging.h"

namespace Filesystem {
    DirectoriesCollection::DirectoriesCollection(const QStringList &directories) {
        extractFiles(directories);
    }

    DirectoriesCollection::DirectoriesCollection(const QList<QUrl> &urls) {
        QStringList directories;
        directories.reserve(urls.length());

        for (auto &url: urls) {
            if (url.isLocalFile()) {
                directories.append(url.toLocalFile());
            } else {
                directories.append(url.path());
            }
        }

        extractFiles(directories);
    }

    DirectoriesCollection::DirectoriesCollection(std::initializer_list<QString> directories):
        DirectoriesCollection(QStringList(directories))
    {
    }

    void DirectoriesCollection::extractFiles(const QStringList &directories) {
        LOG_DEBUG << directories.size() << "directories";
        QStringList files;

        for (auto &directory: directories) {
            QDir dir(directory);

            dir.setFilter(QDir::NoDotAndDotDot | QDir::Files);

            QFileInfoList items = dir.entryInfoList();
            const int size = items.size();
            files.reserve(files.size() + size);

            for (int i = 0; i < size; ++i) {
                QString filepath = items.at(i).absoluteFilePath();
                files.append(filepath);
            }
        }

        const bool fullDirectory = true;
        m_Files = std::make_shared<FilesCollection>(files, fullDirectory);
    }
}
