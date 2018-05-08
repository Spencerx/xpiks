/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbvideocacheindex.h"
#include "../Storage/database.h"
#include "../Helpers/constants.h"
#include "../Storage/memorytable.h"

namespace QMLExtensions {
    DbVideoCacheIndex::DbVideoCacheIndex(Storage::IDatabaseManager *dbManager):
        DbCacheIndex(dbManager)
    {
    }

    bool DbVideoCacheIndex::initialize() {
        LOG_DEBUG << "#";
        Q_ASSERT(m_DatabaseManager != nullptr);

        bool success = false;
        do {
            m_Database = m_DatabaseManager->openDatabase(Constants::VIDEOCACHE_DB_NAME);
            if (!m_Database) {
                LOG_WARNING << "Failed to open database";
            } else {
                m_DbCacheIndex = m_Database->getTable(Constants::VIDEO_CACHE_TABLE);
            }

            if (!m_DbCacheIndex) {
                LOG_WARNING << "Failed to get table" << Constants::VIDEO_CACHE_TABLE;
                m_DbCacheIndex.reset(new Storage::MemoryTable(Constants::VIDEO_CACHE_TABLE));
            }

            success = true;
        } while (false);

        return success;
    }

    void DbVideoCacheIndex::update(const QString &originalPath, CachedVideo &cachedImage) {
        LOG_DEBUG << originalPath;
        CachedVideo previous;

        if (tryGet(originalPath, previous)) {
            cachedImage.m_RequestsServed = previous.m_RequestsServed + 1;
        } else {
            cachedImage.m_RequestsServed = 1;
        }

        insert(originalPath, cachedImage);
    }

    int DbVideoCacheIndex::getMaxCacheMemorySize() const {
#ifdef QT_DEBUG
        return 10;
#else
        return 10000;
#endif
    }
}
