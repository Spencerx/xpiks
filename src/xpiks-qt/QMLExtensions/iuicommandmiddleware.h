/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IUICOMMANDMIDDLEWARE_H
#define IUICOMMANDMIDDLEWARE_H

#include <memory>

namespace QMLExtensions {
    struct UIAction;
}

namespace QMLExtensions {
    class IUICommandMiddlware {
    public:
        virtual ~IUICommandMiddlware() {}
        virtual std::shared_ptr<UIAction> process(std::shared_ptr<UIAction> const &action) = 0;
    };
}

#endif // IUICOMMANDMIDDLEWARE_H
