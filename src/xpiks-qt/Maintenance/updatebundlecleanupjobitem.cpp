/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "updatebundlecleanupjobitem.h"
#include <QDirIterator>
#include "../Common/defines.h"

namespace Maintenance {
    UpdateBundleCleanupJobItem::UpdateBundleCleanupJobItem(const QString &updatesDirectoryPath):
        m_UpdatesDirectoryPath(updatesDirectoryPath)
    {
    }

    void UpdateBundleCleanupJobItem::processJob() {
        LOG_DEBUG << "Working dir:" << m_UpdatesDirectoryPath;

        QDirIterator dirIt(m_UpdatesDirectoryPath, QStringList() << "*.zip" << "*.dmg" << "*.AppImage",
                           QDir::Files, QDirIterator::NoIteratorFlags);
        int count = 0;

        while (dirIt.hasNext()) {
            QString filePath = dirIt.next();
            QFile updateFile(filePath);

            if (updateFile.remove()) {
                LOG_INFO << "Removed prev update bundle:" << filePath;
            } else {
                LOG_WARNING << "Failed to remove bundle:" << filePath;
            }

            count++;
        }

        if (count == 0) {
            LOG_DEBUG << "No update bundles found";
        }
    }
}
