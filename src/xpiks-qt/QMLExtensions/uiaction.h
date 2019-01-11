/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UIACTION_H
#define UIACTION_H

#include <QVariant>

namespace QMLExtensions {
    struct UIAction {
        UIAction(int commandID, QVariant const &value):
            m_CommandID(commandID),
            m_Value(value)
        {}

        int m_CommandID;
        QVariant m_Value;
    };
}

#endif // UIACTION_H
