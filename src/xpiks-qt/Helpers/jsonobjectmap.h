/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef JSONOBJECTMAP_H
#define JSONOBJECTMAP_H

#include <QJsonObject>
#include <QJsonValue>

namespace Helpers {
    class JsonObjectMap
    {
    public:
        JsonObjectMap() { }

        JsonObjectMap(const QJsonObject &json):
            m_Json(json)
        { }

    public:
        QJsonObject json() const { return m_Json; }

    public:
        inline void setValue(const char *key, const QJsonValue &value) {
            m_Json.insert(QLatin1String(key), value);
        }

        inline QJsonValue value(const char *key, const QJsonValue &defaultValue = QJsonValue()) const {
            QJsonValue value = m_Json.value(QLatin1String(key));

            if (value.isUndefined()) {
                return defaultValue;
            }

            return value;
        }

        inline bool boolValue(const char *key, const bool defaultValue = false) const {
            return m_Json.value(QLatin1String(key)).toBool(defaultValue);
        }

        inline double doubleValue(const char *key, const double defaultValue = 0) const {
            return m_Json.value(QLatin1String(key)).toDouble(defaultValue);
        }

        inline int intValue(const char *key, const int defaultValue = 0) const {
            return m_Json.value(QLatin1String(key)).toInt(defaultValue);
        }

        inline QString stringValue(const char *key, const QString &defaultValue = QString("")) const {
            return m_Json.value(QLatin1String(key)).toString(defaultValue);
        }

        inline void deleteValue(const char *key) { m_Json.remove(QLatin1String(key)); }
        inline bool containsValue(const char *key) { return m_Json.contains(QLatin1String(key)); }

    private:
        QJsonObject m_Json;
    };
}

#endif // JSONOBJECTMAP_H
