/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LINQHELPERS_H
#define LINQHELPERS_H

#include <functional>
#include <vector>

namespace Helpers {
    template<typename InType, typename OutType>
    std::vector<OutType> filterMap(const std::vector<InType> &v,
                                   const std::function<bool (const InType&)> &pred,
                                   const std::function<OutType (const InType&)> &mapper) {
        std::vector<OutType> result;
        result.reserve(v.size());
        auto itEnd = v.end();
        auto it = v.begin();
        while (it != itEnd) {
            if (pred(*it)) {
                result.emplace_back(mapper(*it));
            }
            it++;
        }
        return result;
    }

    template<typename InType, typename OutType>
    std::vector<OutType> map(const std::vector<InType> &v, const std::function<OutType (const InType&)> &mapper) {
        std::function<bool(const InType&)> everything = [](const InType&) {return true;};
        return filterMap(v, everything, mapper);
    }

    template<typename T>
    std::vector<T> filter(const std::vector<T> &v, const std::function<bool (const T&)> &pred) {
        std::function<T(const T&)> eye = [](const T &t) { return t; };
        return filterMap(v, pred, eye);
    }
}

#endif // LINQHELPERS_H
