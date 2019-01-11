/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "metadatacache.h"

#include <QChar>
#include <QDataStream>
#include <QIODevice>
#include <QMutexLocker>
#include <QStringList>
#include <Qt>
#include <QtDebug>

#include "Artworks/artworkmetadata.h"
#include "Common/logging.h"
#include "Helpers/constants.h"
#include "MetadataIO/cachedartwork.h"
#include "Microstocks/searchquery.h"
#include "Storage/idatabase.h"
#include "Storage/idatabasemanager.h"
#include "Storage/memorytable.h"

namespace MetadataIO {
    CachedArtwork::CachedArtworkType queryFlagToCachedType(const Microstocks::SearchQuery &query) {
        CachedArtwork::CachedArtworkType searchType = CachedArtwork::Unknown;

        if (query.getSearchPhotos()) {
            searchType = CachedArtwork::Image;
        } else if (query.getSearchVectors()) {
            searchType = CachedArtwork::Vector;
        } else if (query.getSearchVideos()) {
            searchType = CachedArtwork::Video;
        }

        return searchType;
    }

    MetadataCache::MetadataCache(Storage::IDatabaseManager &dbManager):
        m_DatabaseManager(dbManager)
    {
    }

    bool MetadataCache::initialize() {
        LOG_DEBUG << "#";

        bool success = false;
        do {
            m_Database = m_DatabaseManager.openDatabase(Constants::METADATA_CACHE_DB_NAME);
            if (!m_Database) {
                LOG_WARNING << "Failed to open database";
            } else {
                m_DbCacheIndex = m_Database->getTable(Constants::METADATA_CACHE_TABLE);
            }

            if (!m_DbCacheIndex) {
                LOG_WARNING << "Failed to get table" << Constants::METADATA_CACHE_TABLE;
                m_DbCacheIndex = std::make_shared<Storage::MemoryTable>(Constants::METADATA_CACHE_TABLE);
            }

            success = true;
            LOG_INFO << "Metadata cache initialized";
        } while (false);

        return success;
    }

    void MetadataCache::finalize() {
        LOG_DEBUG << "#";

        if (m_Database) {
            m_Database->close();
        }
    }

    void MetadataCache::sync() {
        LOG_DEBUG << "#";

        flushWAL();

        if (m_Database) {
            m_Database->sync();
        }
    }

#ifdef QT_DEBUG
    void MetadataCache::dumpToLog() {
        m_DbCacheIndex->foreachRow([&](const QByteArray &, QByteArray &rawValue) {
            CachedArtwork value;
            QDataStream ds(&rawValue, QIODevice::ReadOnly);
            ds >> value;

            LOG_DEBUG << value.m_Filepath << "|" << value.m_Title << "|" << value.m_Description << "|" << value.m_Keywords;
            return true; // just continue
        });
    }

    void MetadataCache::dumpToArray(QVector<MetadataIO::CachedArtwork> &cachedArtworks) {
        m_DbCacheIndex->foreachRow([&](const QByteArray &, QByteArray &rawValue) {
            CachedArtwork value;
            QDataStream ds(&rawValue, QIODevice::ReadOnly);
            ds >> value;

            cachedArtworks.push_back(value);
            return true; // just continue
        });
    }

    int MetadataCache::retrieveRecordsCount() {
        int count = 0;

        // this craziness is used only for tests
        // in order not to create yet another query
        // in sqlite class used only for 1 case
        m_DbCacheIndex->foreachRow([&count](const QByteArray &, QByteArray &) {
            count++;
            return true; // just continue
        });

        return count;
    }

#endif

    bool MetadataCache::read(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork, CachedArtwork &cachedArtwork) {
        Q_ASSERT(artwork != nullptr);
        if (artwork == nullptr) { return false; }
        if (!m_DbCacheIndex) { return false; }

        const QString &filepath = artwork->getFilepath();
        QByteArray rawValue;
        bool found = false;

        {
            const QByteArray key = filepath.toUtf8();

            QMutexLocker locker(&m_ReadMutex);
            Q_UNUSED(locker);

            found = m_DbCacheIndex->tryGetValue(key, rawValue);
        }

        if (found) {
            CachedArtwork value;
            QDataStream ds(&rawValue, QIODevice::ReadOnly);
            ds >> value;
            Q_ASSERT(ds.status() == QDataStream::Ok);

            if (ds.status() == QDataStream::Ok) {
                cachedArtwork = value;
            }
        }

        return found;
    }

    void MetadataCache::save(const std::shared_ptr<Artworks::ArtworkMetadata> &artwork, bool overwrite) {
        Q_ASSERT(artwork != nullptr);
        if (artwork == nullptr) { return; }
        if (!m_DbCacheIndex) { return; }

        CachedArtwork value(artwork);
        const QString &key = artwork->getFilepath();

        if (overwrite) {
            m_SetWAL.set(key, value);
        } else {
            m_AddWal.set(key, value);
        }
    }

    void MetadataCache::search(const Microstocks::SearchQuery &query, QVector<CachedArtwork> &results) {
        Q_ASSERT(results.empty());
        QStringList searchTerms = query.getSearchQuery().split(QChar::Space, QString::SkipEmptyParts);
        LOG_VERBOSE_OR_DEBUG << searchTerms;
        if (!m_DbCacheIndex) { return; }
        CachedArtwork::CachedArtworkType searchType = queryFlagToCachedType(query);

        m_DbCacheIndex->foreachRow([&](const QByteArray &, QByteArray &rawValue) {
            CachedArtwork value;
            QDataStream ds(&rawValue, QIODevice::ReadOnly);
            ds >> value;

            LOG_VERBOSE << value.m_Filepath << "|" << value.m_Title << "|" << value.m_Description << "|" << value.m_Keywords;

            if (ds.status() != QDataStream::Ok) { /*continue;*/ return true; }
            if ((searchType != CachedArtwork::Unknown) && (value.m_ArtworkType != searchType)) { /*continue;*/ return true; }

            bool hasMatch = false;

            foreach (const QString &searchTerm, searchTerms) {
                if (value.m_Title.contains(searchTerm, Qt::CaseInsensitive)) {
                    hasMatch = true;
                    break;
                }

                if (value.m_Description.contains(searchTerm, Qt::CaseInsensitive)) {
                    hasMatch = true;
                    break;
                }

                foreach (const QString &keyword, value.m_Keywords) {
                    if (keyword.contains(searchTerm, Qt::CaseInsensitive)) {
                        hasMatch = true;
                        break;
                    }
                }

                if (hasMatch) { break; }
            }

            if (hasMatch) {
                results.push_back(value);
            }

            const bool canContinue = results.size() < query.getPageSize();
            return canContinue;
        });

        LOG_DEBUG << "Found" << results.size() << "matches";
    }

    void MetadataCache::flushWAL() {
        LOG_DEBUG << "#";
        if (!m_DbCacheIndex) { return; }

        LOG_DEBUG << "Add WAL size:" << m_AddWal.size();
        LOG_DEBUG << "Set WAL size:" << m_SetWAL.size();

        m_AddWal.flush(m_DbCacheIndex);
        m_SetWAL.flush(m_DbCacheIndex);
    }
}
