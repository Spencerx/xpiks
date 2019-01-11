/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <utility>

namespace Common {
    typedef uint32_t flag_t;

    template <typename T, int Tag=0>
    class NamedType
    {
    public:
        NamedType(T const &value) : m_Value(value) {}
        NamedType(T &&value) : m_Value(std::move(value)) {}
        NamedType(const NamedType &other): m_Value(other.m_Value) {}
        T& get() { return m_Value; }
        T const& get() const { return m_Value; }
        bool operator==(NamedType<T> const &t) const {
            return m_Value == t.m_Value;
        }
        bool operator!=(NamedType<T> const &t) const {
            return m_Value != t.m_Value;
        }

    private:
        T m_Value;
    };

    using ID_t = NamedType<uint32_t>;
}

#endif // TYPES_H
