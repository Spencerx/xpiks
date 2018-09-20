/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef STATEFULENTITY_H
#define STATEFULENTITY_H

#include "../Helpers/localconfig.h"
#include <QString>
#include <QAtomicInt>
#include <QJsonObject>
#include <memory>
#include "isystemenvironment.h"
#include "../Helpers/jsonobjectmap.h"

namespace Common {
    class StatefulEntity
    {
    public:
        StatefulEntity(const QString &stateName, ISystemEnvironment &environment);
        virtual ~StatefulEntity();

    public:
        void init();
        void sync();

    public:
        inline void setValue(const char *key, const QJsonValue &value) {
            m_StateMap->setValue(key, value);
        }

        inline QJsonValue getValue(const char *key, const QJsonValue &defaultValue = QJsonValue()) const {
            return m_StateMap->value(key, defaultValue);
        }

        inline bool getBool(const char *key, const bool defaultValue = false) const {
            return m_StateMap->boolValue(key, defaultValue);
        }

        inline double getDouble(const char *key, const double defaultValue = 0) const {
            return m_StateMap->doubleValue(key, defaultValue);
        }

        inline int getInt(const char *key, const int defaultValue = 0) const {
            return m_StateMap->intValue(key, defaultValue);
        }

        inline QString getString(const char *key, const QString &defaultValue = QString("")) const {
            return m_StateMap->stringValue(key, defaultValue);
        }

        inline bool contains(const char *key) const {
            return m_StateMap->containsValue(key);
        }

        inline void remove(const char *key) const {
            return m_StateMap->deleteValue(key);
        }

    private:
        QString m_StateName;
        Helpers::LocalConfig m_Config;
        std::shared_ptr<Helpers::JsonObjectMap> m_StateMap;
        QAtomicInt m_InitCounter;
    };
}

#endif // STATEFULENTITY_H
