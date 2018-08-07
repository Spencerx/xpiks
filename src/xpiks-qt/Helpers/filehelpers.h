/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef FILENAMESHELPERS
#define FILENAMESHELPERS

#include <QStringList>

namespace Helpers {
    QStringList convertToVectorFilenames(const QStringList &items);
    QStringList convertToVectorFilenames(const QString &path);
    QString getImagePath(const QString &path);
    QString getArchivePath(const QString &artworkPath);
    bool couldBeVideo(const QString &artworkPath);
    bool isVideoExtension(const QString &extension);
    bool isVectorExtension(const QString &extension);
    bool isImageExtension(const QString &extension);
    bool isSupportedExtension(const QString &extension);
    QString describeFileSize(qint64 filesizeBytes);
    bool ensureDirectoryExists(const QString &path);
    void splitMediaFiles(const QStringList &rawFilenames, QStringList &filenames, QStringList &vectors);
}

#endif // FILENAMESHELPERS

