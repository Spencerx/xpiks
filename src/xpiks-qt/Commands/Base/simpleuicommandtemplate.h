/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SIMPLEUICOMMANDTEMPLATE_H
#define SIMPLEUICOMMANDTEMPLATE_H

#include "iuicommandtemplate.h"
#include <functional>

namespace Commands {
    class SimpleUICommandTemplate: public IUICommandTemplate
    {
        using SimpleCallback = std::function<void(const QJSValue &value)>;

    public:
        SimpleUICommandTemplate(const SimpleCallback &callback):
            m_Callback(callback)
        { }

        // IUICommandTemplate interface
    public:
        virtual void execute(const QJSValue &value) override {
            m_Callback(value);
        }

    private:
         SimpleCallback m_Callback;
    };
}

#endif // SIMPLEUICOMMANDTEMPLATE_H
