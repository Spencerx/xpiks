/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>

namespace Commands {
    template<class... Args>
    class Message {
    public:
        Message(int id):
            m_ID(id)
        {
        }

    public:
        int getID() const { return m_ID; }

    public:
        void broadcast(Args&&... args) {
            for (auto &listener: m_Listeners) {
                listener(std::forward<Args>(args)...);
            }
        }

        void addListener(std::function<void(Args...)> listener) {
            m_Listeners.emplace_back(listener);
        }

    private:
        std::vector<std::function<void(Args...)>> m_Listeners;
        int m_ID;
    };

    template<class... Args>
    class MessageMap {
    public:
        Message<Args...> &withID(int id) {
            auto it = m_Messages.find(id);
            if (it == m_Messages.end()) {
                it = m_Messages.emplace(id, id).first;
            }
            return it->second;
        }

    private:
        std::unordered_map<int, Message<Args...>> m_Messages;
    };

    template<class... Hubs>
    class RegisteredMessages: private Hubs...
    {
    public:
        template<class... Args>
        MessageMap<Args...> &ofType() {
            return *this;
        }
    };
}

#endif // MESSAGES_H
