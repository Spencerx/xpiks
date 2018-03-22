/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef METADATACACHE_H
#define METADATACACHE_H

#include <QMutex>
#include <QReadWriteLock>
#include "../Storage/idatabase.h"
#include "cachedartwork.h"
#include "../Suggestion/searchquery.h"
#include "../Storage/writeaheadlog.h"

namespace Models {
    class ArtworkMetadata;
}

namespace Storage {
    class IDatabaseManager;
}

namespace MetadataIO {
    class ArtworkSetWAL: public Storage::WriteAheadLog<QString, CachedArtwork> {
    protected:
        virtual QByteArray keyToByteArray(const QString &key) const override { return key.toUtf8(); }
        virtual QString keyFromByteArray(const QByteArray &rawKey) const override { return QString::fromUtf8(rawKey); }
        virtual bool doFlush(std::shared_ptr<Storage::IDbTable> &dbTable, const QVector<QPair<QByteArray, QByteArray> > &keyValuesList, QVector<int> &failedIndices) override {
            return dbTable->trySetMany(keyValuesList, failedIndices);
        }
    };

    class ArtworkAddWAL: public Storage::WriteAheadLog<QString, CachedArtwork> {
    protected:
        virtual QByteArray keyToByteArray(const QString &key) const override { return key.toUtf8(); }
        virtual QString keyFromByteArray(const QByteArray &rawKey) const override { return QString::fromUtf8(rawKey); }
        virtual bool doFlush(std::shared_ptr<Storage::IDbTable> &dbTable, const QVector<QPair<QByteArray, QByteArray> > &keyValuesList, QVector<int> &failedIndices) override {
            Q_UNUSED(failedIndices);
            int count = dbTable->tryAddMany(keyValuesList);
            return count > 0;
        }
    };

    class MetadataCache
    {
    public:
        MetadataCache(Storage::IDatabaseManager *dbManager);

    public:
        bool initialize();
        void finalize();
        void sync();

#ifdef QT_DEBUG
    public:
        void dumpToLog();
        void dumpToArray(QVector<MetadataIO::CachedArtwork> &cachedArtworks);
        int retrieveRecordsCount();
#endif

    public:
        bool read(Models::ArtworkMetadata *artwork, CachedArtwork &cachedArtwork);
        void save(Models::ArtworkMetadata *metadata, bool overwrite = true);

    public:
        void search(const Suggestion::SearchQuery &query, QVector<CachedArtwork> &results);

    private:
        void flushWAL();

    private:
        QMutex m_ReadMutex;
        Storage::IDatabaseManager *m_DatabaseManager;
        std::shared_ptr<Storage::IDbTable> m_DbCacheIndex;
        std::shared_ptr<Storage::IDatabase> m_Database;
        ArtworkSetWAL m_SetWAL;
        ArtworkAddWAL m_AddWal;
    };
}

#endif // METADATACACHE_H
