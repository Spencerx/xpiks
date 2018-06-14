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
        m_Images.reserve(urls.length());
        m_Vectors.reserve(urls.length()/2);
        m_Videos.reserve(urls.length()/2);

        QStringList files;
        files.reserve(urls.length());

        for (const auto &url: urls) {
            files.append(url.toLocalFile());
        }

        sortRawFiles(files);
    }

    FilesCollection::FilesCollection(const QStringList &files) {
        m_Images.reserve(files.length());
        m_Vectors.reserve(files.length()/2);
        m_Videos.reserve(files.length()/2);

        sortRawFiles(files);
    }

    void FilesCollection::sortRawFiles(const QStringList &files) {
        LOG_INFO << files.length() << "file(s)";

        foreach(const QString &filepath, files) {
            QFileInfo fi(filepath);
            const QString suffix = fi.suffix().toLower();

            if (Helpers::isImageExtension(suffix)) {
                m_Images.append(filepath);
            } else if (Helpers::isVectorExtension(suffix)) {
                m_Vectors.append(filepath);
            } else if (Helpers::isVideoExtension(suffix)) {
                m_Videos.append(filepath);
            } else if (suffix == QLatin1String("png")) {
                LOG_WARNING << "PNG is unsupported file format";
            } else if (suffix != QLatin1String(Constants::METADATA_BACKUP_SUFFIX)) {
                LOG_WARNING << "Unsupported extension of file" << filepath;
            }
        }
    }

}
