/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

namespace Common {
    typedef uint32_t flag_t;

    template <typename T, typename Parameter=int>
    class NamedType
    {
    public:
        explicit NamedType(T const &value) : m_Value(value) {}
        explicit NamedType(T &&value) : m_Value(std::move(value)) {}
        T& get() { return m_Value; }
        T const& get() const { return m_Value; }
    private:
        T m_Value;
    };

    using ID_t = NamedType<uint32_t>;
}

#endif // TYPES_H
