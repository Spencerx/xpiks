/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IACTIONMODEL_H
#define IACTIONMODEL_H

#include <memory>

namespace Commands {
    class ICommand;
}

namespace Models {
    class IActionModel {
    public:
        virtual ~IActionModel() {}
        virtual std::shared_ptr<Commands::ICommand> getActionCommand(bool yesno) = 0;
        virtual void resetModel() = 0;
    };
}

#endif // IACTIONMODEL_H
