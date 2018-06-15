/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "filescollection.h"
#include <QFileInfo>
#include "../Helpers/filehelpers.h"
#include "../Common/logging.h"
#include "../Helpers/constants.h"

namespace Filesystem {
    FilesCollection::FilesCollection(const QList<QUrl> &urls) {
        m_Files.reserve(urls.length());

        QStringList files;
        files.reserve(urls.length());

        for (const auto &url: urls) {
            files.append(url.toLocalFile());
        }

        sortRawFiles(files);
    }

    FilesCollection::FilesCollection(const QStringList &files) {
        m_Files.reserve(files.length());
        sortRawFiles(files);
    }

    void FilesCollection::sortRawFiles(const QStringList &files) {
        LOG_INFO << files.length() << "file(s)";

        foreach(const QString &filepath, files) {
            QFileInfo fi(filepath);
            const QString suffix = fi.suffix().toLower();

            if (Helpers::isImageExtension(suffix)) {
                m_Files.emplace_back(filepath, Filesystem::ArtworkFileType::Image);
            } else if (Helpers::isVectorExtension(suffix)) {
                m_Files.emplace_back(filepath, Filesystem::ArtworkFileType::Vector);
            } else if (Helpers::isVideoExtension(suffix)) {
                m_Files.emplace_back(filepath, Filesystem::ArtworkFileType::Video);
            } else if (suffix == QLatin1String("png")) {
                LOG_WARNING << "PNG is unsupported file format";
            } else if (suffix != QLatin1String(Constants::METADATA_BACKUP_SUFFIX)) {
                LOG_WARNING << "Unsupported extension of file" << filepath;
            }
        }
    }

}
