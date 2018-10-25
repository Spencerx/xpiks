/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "obfuscation.h"

#include <QChar>
#include <QCharRef>
#include <QtGlobal>

namespace Encryption {
    QString rot13plus(const QString &line) {
        QString result = line;

        const int size = line.size();
        for (int i = 0; i < size; i++) {
            QChar c = result[i];
            ushort u = c.unicode();

            if (c >= QChar('!') && c <= QChar('~')) {
                u = '!' + (u - '!' + 47) % 94;
            }

            result[i] = QChar(u);
        }

        return result;
    }
}
