/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORIGINALMETADATA_H
#define ORIGINALMETADATA_H

#include <QStringList>
#include <QString>
#include <QSize>
#include <QHash>
#include <QDateTime>

namespace MetadataIO {
    // metadata read directly from file by exiftool/etc.
    struct OriginalMetadata {
        OriginalMetadata():
            m_ImageSize(0, 0),
            m_FileSize(0),
            m_DateTimeOriginal(),
            m_VideoFrameRate(0.0),
            m_VideoBitRate(0),
            m_VideoDuration(0.0)
        { }

        QString m_FilePath;
        QString m_Title;
        QString m_Description;
        QStringList m_Keywords;
        QSize m_ImageSize;
        qint64 m_FileSize;

        /*PHOTO*/QDateTime m_DateTimeOriginal;

        /*VIDEO*/double m_VideoFrameRate;
        /*VIDEO*/qint64 m_VideoBitRate;
        /*VIDEO*/double m_VideoDuration;
    };
}

#endif // IMPORTDATARESULT_H
