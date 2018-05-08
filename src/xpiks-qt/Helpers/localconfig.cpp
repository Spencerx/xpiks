/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "localconfig.h"
#include <QDir>
#include <QStandardPaths>

namespace Helpers {
    LocalConfig::LocalConfig(const QString &filepath, bool memoryOnly):
        m_FilePath(filepath),
        m_MemoryOnly(memoryOnly)
    {
        Q_ASSERT(!m_FilePath.isEmpty());
    }

    void LocalConfig::initialize() {
        QFile file(m_FilePath);

        if (file.open(QIODevice::ReadOnly)) {
            QString text = QString::fromUtf8(file.readAll());
            file.close();
            m_Config = QJsonDocument::fromJson(text.toUtf8());
        } else {
            LOG_WARNING << "Opening file" << m_FilePath << "failed";
        }
    }

    bool LocalConfig::save() {
        LOG_DEBUG << "memory-only:" << m_MemoryOnly;

        if (m_MemoryOnly) { return true; }
        if (m_FilePath.isEmpty()) { return false; }

        bool success = false;
        QFile file(m_FilePath);

        if (file.open(QIODevice::WriteOnly)) {
            file.write(m_Config.toJson(QJsonDocument::Indented));
            file.close();
            success = true;
        } else {
            LOG_WARNING << "Opening file" << m_FilePath << "failed";
        }

        return success;
    }

    void LocalConfig::dropConfig() {
        if (m_MemoryOnly) { return; }
        m_Config = QJsonDocument();
        Q_ASSERT(m_Config.isEmpty());
    }
}

