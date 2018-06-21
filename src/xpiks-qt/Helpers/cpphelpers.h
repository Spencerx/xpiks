/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LINQHELPERS_H
#define LINQHELPERS_H

#include <vector>
#include <functional>
#include <algorithm>

namespace Helpers {
    template<typename InType, typename OutType>
    std::vector<OutType> map(const std::vector<InType> &v, const std::function<OutType (const InType&)> &mapper) {
        std::vector<OutType> result;
        result.reserve(v.size());
        auto itEnd = v.end();
        auto it = v.begin();
        while (it != itEnd) {
            result.emplace_back(mapper(*it++));
        }
        return result;
    }

    template<typename T>
    std::vector<T> filter(const std::vector<T> &v, const std::function<bool (const T&)> &pred) {
        std::vector<T> result;
        result.reserve(v.size());
        auto itEnd = v.end();
        auto it = v.begin();
        while (it != itEnd) {
            if (pred(*it)) {
                result.push_back(*it);
            }
            it++;
        }
        return result;
    }
}

#endif // LINQHELPERS_H
