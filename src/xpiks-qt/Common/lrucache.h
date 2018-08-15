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

#include <unordered_map>
#include <algorithm>
#include <vector>
#include <utility>

namespace Common {
    template<typename TKey, typename TValue>
    class LRUCache {
    private:
        struct CachedItem {
            CachedItem(TKey key, TValue value, size_t tag):
                m_Key(key),
                m_Value(value),
                m_CacheTag(tag)
            {
            }

            TKey m_Key;
            TValue m_Value;
            size_t m_CacheTag;
        };

    public:
        LRUCache(size_t size):
            m_CacheSize(size),
            m_LatestTag(0)
        { }

    public:
        size_t size() { return m_Data.size(); }

        TValue const &get(TKey const &key, TValue const &defaultValue = TValue()) {
            auto it = m_Map.find(key);
            if (it != m_Map.end()) {
                auto &item = m_Data.at(it->second);
                touch(item);
                return item.m_Value;
            } else {
                return defaultValue;
            }
        }

        void put(TKey const &key, TValue const &value) {
            auto it = m_Map.find(key);
            if (it == m_Map.end()) {
                m_Data.emplace_back(key, value, nextTag());
                m_Map.insert({key, (size_t)(m_Data.size() - 1)});
                cleanup();
            } else {
                auto &item = m_Data[it->second];
                item.m_Value = value;
                touch(item);
            }
        }

    private:
        void touch(CachedItem &item) {
            if (item.m_CacheTag != (m_LatestTag - 1)) {
                item.m_CacheTag = nextTag();
            }
        }

        size_t nextTag() {
            return m_LatestTag++;
        }

        void cleanup() {
            if (m_Data.size() <= m_CacheSize) { return; }

            m_Data.erase(
                        std::remove_if(m_Data.begin(), m_Data.end(),
                                       [this](CachedItem const &item) {
                             return (this->m_LatestTag - item.m_CacheTag) > m_CacheSize;
                        }),
                    m_Data.end());

            m_Map.clear();
            const size_t size = m_Data.size();
            for (size_t i = 0; i < size; i++) {
                m_Map.insert({m_Data[i].m_Key, i});
            }
        }

    private:
        std::vector<CachedItem> m_Data;
        std::unordered_map<TKey, size_t> m_Map;
        size_t m_CacheSize;
        size_t m_LatestTag;
    };
}

#endif // LRUCACHE_H
