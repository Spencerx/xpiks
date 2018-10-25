/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INDICESHELPER_H
#define INDICESHELPER_H

#include <utility>
#include <vector>

template <typename T> class QVector;

namespace Helpers {
    typedef std::vector<std::pair<int, int> > RangesVector;

    template<class T>
    int splitIntoChunks(const QVector<T> &items, int chunksCount, QVector<QVector<T> > &chunks);
    RangesVector unionRanges(RangesVector &ranges);
}

#endif // INDICESHELPER_H
