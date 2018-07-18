/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHANGESLISTENER_H
#define CHANGESLISTENER_H

#include <vector>

namespace Common {
    template<typename T>
    class EventsTarget {
    public:
        virtual ~EventsTarget() {}
        virtual void handleChange(const T &change) = 0;
    };

    template<typename T>
    class EventsSource {
    public:
        virtual ~EventsSource() {}

    public:
        void addTarget(EventsTarget<T> &&target) {
            m_Targets.emplace_back(target);
        }

        void notifyChange(const T &change) {
            for (auto &target: m_Targets) {
                target.handleChange(change);
            }
        }

    private:
        std::vector<EventsTarget<T>> m_Targets;
    };
}

#endif // CHANGESLISTENER_H
