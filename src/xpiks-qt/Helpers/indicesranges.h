/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INDICESRANGES_H
#define INDICESRANGES_H

#include <utility>
#include <vector>
#include <QVector>
#include <initializer_list>

namespace Helpers {
    class IndicesRanges {
    public:
        IndicesRanges() { }
        IndicesRanges(std::initializer_list<int> l);
        IndicesRanges(std::vector<int> const &indices);
        IndicesRanges(QVector<int> const &indices);
        IndicesRanges(int start, int count);
        IndicesRanges(size_t count);
        IndicesRanges(IndicesRanges const &other);
        IndicesRanges(IndicesRanges &&other);
        IndicesRanges &operator=(IndicesRanges &&other);
        IndicesRanges &operator=(IndicesRanges const &other);

#ifdef CORE_TESTS
        // this one is for tests since others check validity of ranges
        IndicesRanges(std::initializer_list<std::pair<int, int>> ranges);
#endif

    public:
        const std::vector<std::pair<int, int> > &getRanges() const { return m_Ranges; }
        int length() const;
        size_t size() const { return m_Ranges.size(); }
        std::vector<int> retrieveIndices() const;

    private:
        std::vector<std::pair<int, int>> m_Ranges;
    };
}

#endif // INDICESRANGES_H
