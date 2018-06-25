/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMMANDSINK_H
#define COMMANDSINK_H

#include <type_traits>
#include "icommand.h"

namespace Commands {
    class CommandsSink {

    public:
        CommandsSink() {
        }

    public:
        template<class T>
        void process(const std::shared_ptr<T> &command);
    };
}

#endif // COMMANDSINK_H
