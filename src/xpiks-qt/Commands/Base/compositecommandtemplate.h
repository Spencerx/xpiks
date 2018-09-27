/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMPOSITECOMMANDTEMPLATE_H
#define COMPOSITECOMMANDTEMPLATE_H

#include <memory>
#include <vector>
#include <initializer_list>
#include "icommandtemplate.h"

namespace Commands {
    template<typename T>
    class CompositeCommandTemplate: public ICommandTemplate<T>
    {
    public:
        CompositeCommandTemplate(std::initializer_list<std::shared_ptr<ICommandTemplate<T>>> list):
            m_Templates(list)
        {
        }

        CompositeCommandTemplate(std::vector<std::shared_ptr<ICommandTemplate<T>>> &templates):
            m_Templates(std::move(templates))
        {
        }

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(const T &argument) override {
            for (auto &t: m_Templates) {
                if (t) {
                    t->execute(argument);
                }
            }
        }

        virtual void undo(const T &argument) override {
            for (auto &t: m_Templates) {
                if (t) {
                    t->undo(argument);
                }
            }
        }

    private:
        std::vector<std::shared_ptr<ICommandTemplate<T>> > m_Templates;
    };
}

#endif // COMPOSITECOMMANDTEMPLATE_H
