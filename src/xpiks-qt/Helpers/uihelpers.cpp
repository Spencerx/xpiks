/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uihelpers.h"

namespace Helpers {    
    bool convertToBool(QVariant const &value, bool defaultValue) {
        bool result = defaultValue;
        if (value.isValid()) {
            if (value.type() == QVariant::Bool) {
                result = value.toBool();
            }
        }
        return result;
    }

    int convertToInt(QVariant const &value, int defaultValue) {
        int result = defaultValue;
        if (value.isValid()) {
            if (value.type() == QVariant::Int) {
                result = value.toInt();
            }
        }
        return result;
    }

    QString convertToString(QVariant const &value, QString const &defaultValue) {
        QString result = defaultValue;
        if (value.isValid()) {
            if (value.type() == QVariant::String) {
                result = value.toString();
            }
        }
        return result;
    }
}
