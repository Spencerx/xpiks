/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <cstddef>
#include <list>
#include <unordered_map>
#include <utility>

namespace Common {
    template<typename TKey, typename TValue>
    class LRUCache {
        using KeyValuePair = std::pair<TKey, TValue>;
        typedef typename std::list<KeyValuePair>::iterator ListIterator;

    public:
        LRUCache(size_t size):
            LRUCache(size, size/2)
        {
        }

        LRUCache(size_t size, size_t elasticity):
            m_MaxCacheSize(size),
            m_Elasticity(elasticity)
        {
        }

    public:
        size_t size() const { return m_Data.size(); }

        TValue const &get(TKey const &key, TValue const &defaultValue = TValue()) {
            auto it = m_KeysMap.find(key);
            if (it != m_KeysMap.end()) {
                m_Data.splice(m_Data.begin(), m_Data, it->second);
                return it->second->second;
            } else {
                return defaultValue;
            }
        }

        bool tryGet(TKey const &key, TValue &value) {
            bool found = false;
            auto it = m_KeysMap.find(key);
            if (it != m_KeysMap.end()) {
                m_Data.splice(m_Data.begin(), m_Data, it->second);
                value = it->second->second;
                found = true;
            }
            return found;
        }

        void put(TKey const &key, TValue const &value) {
            const auto it = m_KeysMap.find(key);
            if (it != m_KeysMap.end()) {
                it->second->second = value;
                m_Data.splice(m_Data.begin(), m_Data, it->second);
            } else {
                m_Data.emplace_front(key, value);
                m_KeysMap[key] = m_Data.begin();

                if (m_Data.size() >= (m_MaxCacheSize + m_Elasticity)) {
                    normalize();
                }
            }
        }

        void clear() {
            m_KeysMap.clear();
            m_Data.clear();
        }

        void normalize() {
            Q_ASSERT(m_Data.size() >= m_KeysMap.size());
            while (m_Data.size() > m_MaxCacheSize) {
                m_KeysMap.erase(m_Data.back().first);
                m_Data.pop_back();
            }
        }

    private:
        std::unordered_map<TKey, ListIterator> m_KeysMap;
        std::list<KeyValuePair> m_Data;
        size_t m_MaxCacheSize;
        size_t m_Elasticity;
    };
}

#endif // LRUCACHE_H
