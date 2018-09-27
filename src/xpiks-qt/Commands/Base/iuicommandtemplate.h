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

#include <QVariant>
#include <vector>

namespace Commands {
    class IUICommandTemplate {
    public:
        virtual ~IUICommandTemplate() {}
        virtual int getCommandID() = 0;
        virtual void execute(QVariant const &value) = 0;
        virtual void undo(QVariant const &) { /*BUMP*/ }
        virtual bool canUndo() { return false; }
        virtual QString getDescription() const { return QString(); }
    };
}

#endif // IUICOMMANDTEMPLATE_H
