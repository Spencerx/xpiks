/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SIMPLECOMMAND_H
#define SIMPLECOMMAND_H

#include "icommand.h"
#include <functional>

namespace Commands {
    class SimpleCommand: public ICommand
    {
        using SimpleCallback = std::function<void()>;

    public:
        SimpleCommand(const SimpleCallback &callback):
            m_Callback(callback)
        { }

    public:
        virtual void execute() override {
            m_Callback();
        }

    private:
         SimpleCallback m_Callback;
    };
}

#endif // SIMPLECOMMAND_H
