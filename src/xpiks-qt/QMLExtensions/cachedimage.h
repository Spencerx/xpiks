/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CACHEDIMAGE_H
#define CACHEDIMAGE_H

#include <QDateTime>
#include <QSize>
#include <QString>
#include <QtGlobal>

class QDataStream;

namespace QMLExtensions {
    struct CachedImage {
        CachedImage();
        CachedImage(const CachedImage &from);
        CachedImage &operator=(const CachedImage &other);

        quint32 m_Version; // 32-bit for allignment
        // BEGIN of data version 1
        QDateTime m_LastModified;
        QString m_Filename;
        QSize m_Size;
        quint64 m_RequestsServed;
        // END of data version 1
    };

    QDataStream &operator<<(QDataStream &out, const CachedImage &v);
    QDataStream &operator>>(QDataStream &in, CachedImage &v);
}

#endif // CACHEDIMAGE_H
