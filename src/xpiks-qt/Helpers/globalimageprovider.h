/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GLOBALIMAGEPROVIDER_H
#define GLOBALIMAGEPROVIDER_H

#include <QImage>
#include <QObject>
#include <QQmlEngine>
#include <QQuickImageProvider>
#include <QString>

class QSize;

namespace Helpers {
    class GlobalImageProvider : public QObject, public QQuickImageProvider
    {
        Q_OBJECT
    public:
        GlobalImageProvider(ImageType type, Flags flags = 0) :
            QQuickImageProvider(type, flags)
        {}

        virtual ~GlobalImageProvider() {}

        virtual QImage requestImage(const QString &url, QSize *size, const QSize& requestedSize) override;
    };
}
#endif // GLOBALIMAGEPROVIDER_H
