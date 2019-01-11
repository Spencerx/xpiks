/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef IUIPROVIDER_H
#define IUIPROVIDER_H

#include <QHash>
#include <QString>

class QObject;
class QUrl;

namespace Plugins {
    class IUIProvider {
    public:
        virtual ~IUIProvider() { }

        virtual void openDialog(const QUrl &rcPath,
                                const QHash<QString, QObject*> &contextModels = QHash<QString, QObject*>()) const = 0;
        virtual int addTab(const QString &tabIconUrl, const QString &tabComponentUrl, QObject *tabModel) const = 0;
        virtual bool removeTab(int tabID) const = 0;
    };
}

#endif // IUIPROVIDER_H
