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
#include <mutex>

namespace Commands {
    template<class... Args>
    class Message {
    public:
        Message(int id):
            m_ID(id)
        {
        }

    public:
        void broadcast(const Args&... args) {
            std::lock_guard<std::mutex> guard(m_ListenersMutex);
            for (auto &listener: m_Listeners) {
                listener(args...);
            }
        }

        void addListener(std::function<void(const Args&...)> listener) {
            std::lock_guard<std::mutex> guard(m_ListenersMutex);
            m_Listeners.emplace_back(listener);
        }

    private:
        std::mutex m_ListenersMutex;
        std::vector<std::function<void(const Args&...)>> m_Listeners;
        int m_ID;
    };

    template<class... Args>
    class MessageMap {
    public:
        Message<Args...> &withID(int id) {
            std::lock_guard<std::mutex> guard(m_MessagesMutex);
            auto it = m_Messages.find(id);
            if (it == m_Messages.end()) {
                it = m_Messages.emplace(id, id).first;
            }
            return it->second;
        }

    private:
        std::mutex m_MessagesMutex;
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
