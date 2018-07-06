/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "indicesranges.h"
#include <algorithm>

namespace Helpers {
    void indicesToRanges(const std::vector<int> &indices, std::vector<std::pair<int, int> > &ranges) {
        if (indices.empty()) { return; }

        int currentStart = indices[0];
        const size_t indicesCount = indices.size();
        ranges.reserve(indicesCount/3);

        for (size_t i = 1; i < indicesCount; ++i) {
            if (indices[i] - indices[i - 1] > 1) {
                ranges.emplace_back(std::make_pair(currentStart, indices[i - 1]));
                currentStart = indices[i];
            }
        }

        ranges.emplace_back(std::make_pair(currentStart, indices[indicesCount - 1]));
    }

    IndicesRanges::IndicesRanges(std::initializer_list<int> l) {
        std::vector<int> sortedIndices;
        sortedIndices.insert(sortedIndices.end(), l.begin(), l.end());
        std::sort(sortedIndices.begin(), sortedIndices.end());
        indicesToRanges(sortedIndices, m_Ranges);
    }

    IndicesRanges::IndicesRanges(const std::vector<int> &indices) {
        std::vector<int> sortedIndices = indices;
        std::sort(sortedIndices.begin(), sortedIndices.end());
        indicesToRanges(sortedIndices, m_Ranges);
    }

    IndicesRanges::IndicesRanges(const QVector<int> &indices) {
        std::vector<int> sortedIndices = indices.toStdVector();
        std::sort(sortedIndices.begin(), sortedIndices.end());
        indicesToRanges(sortedIndices, m_Ranges);
    }

    IndicesRanges::IndicesRanges(int start, int count) {
        m_Ranges.emplace_back(std::make_pair(start, start + count - 1));
    }

    IndicesRanges::IndicesRanges(size_t count) {
        m_Ranges.emplace_back(std::make_pair(0, count - 1));
    }

    IndicesRanges::IndicesRanges(const IndicesRanges &other):
        m_Ranges(other.m_Ranges)
    {
    }

    IndicesRanges::IndicesRanges(IndicesRanges &&other)
    {
        m_Ranges.swap(other.m_Ranges);
    }

    IndicesRanges &IndicesRanges::operator=(IndicesRanges &&other) {
        if (this != &other) {
            m_Ranges.swap(other.m_Ranges);
        }
        return *this;
    }

    int IndicesRanges::length() const {
        int sum = 0;
        for (auto &pair: m_Ranges) {
            Q_ASSERT(pair.second >= pair.first);
            sum += pair.second - pair.first + 1;
        }
        return sum;
    }

    std::vector<int> IndicesRanges::retrieveIndices() const {
        std::vector<int> indices;
        indices.reserve(m_Ranges.size()*3);
        for (auto &pair: m_Ranges) {
            Q_ASSERT(pair.second >= pair.first);
            for (int i = pair.first; i <= pair.second; i++) {
                indices.push_back(i);
            }
        }
        return indices;
    }
}
