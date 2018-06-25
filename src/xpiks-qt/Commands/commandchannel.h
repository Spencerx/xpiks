/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMMANDCHANNEL_H
#define COMMANDCHANNEL_H

#include <atomic>
#include <memory>
#include <unordered_map>
#include "commandsink.h"

namespace Commands {
    class CommandChannel {
        using Container = std::unordered_map<int, CommandsSink>;
    private:
        template <class Key>
        static int type_id() {
            static int id = ++m_LastTypeID;
            return id;
        }

    public:
        template<typename T>
        void send(const std::shared_ptr<T> &command) {

        }

    private:
      static std::atomic<int> m_LastTypeID;

    };


    template <class T>
    std::atomic<int> CommandChannel<T>::m_LastTypeID(0);
}

#endif // COMMANDCHANNEL_H
