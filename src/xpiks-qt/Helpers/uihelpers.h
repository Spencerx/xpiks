/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UIHELPERS_H
#define UIHELPERS_H

#include <QVariant>
#include <QString>

namespace Helpers {
    bool convertToBool(QVariant const &value, bool defaultValue = false);
    int convertToInt(QVariant const &value, int defaultValue = 0);
    QString convertToString(QVariant const &value, QString const &defaultValue = QString(""));
}

#endif // UIHELPERS_H
