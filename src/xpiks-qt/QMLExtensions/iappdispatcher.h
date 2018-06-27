/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IAPPDISPATCHER_H
#define IAPPDISPATCHER_H

#include <memory>

namespace Commands {
    class IUICommandTemplate;
}

namespace QMLExtensions {
    class IAppDispatcher {
    public:
        virtual ~IAppDispatcher() {}
        virtual void registerCommand(int commandID, const std::shared_ptr<Commands::IUICommandTemplate> &command) = 0;
    };
}

#endif // IAPPDISPATCHER_H
