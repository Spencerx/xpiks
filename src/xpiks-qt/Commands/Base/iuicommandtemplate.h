/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IUICOMMANDTEMPLATE_H
#define IUICOMMANDTEMPLATE_H

#include <QJSValue>

namespace Commands {
    class IUICommandTemplate {
    public:
        virtual ~IUICommandTemplate() {}
        virtual void execute(const QJSValue &value) = 0;
        virtual int getCommandID() = 0;
    };
}

#endif // IUICOMMANDTEMPLATE_H
