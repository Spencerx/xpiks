/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef METADATAIOSERVICE_H
#define METADATAIOSERVICE_H

#include <memory>

#include <QObject>
#include <QString>
#include <Qt>
#include <QtGlobal>

#include "Artworks/artworkssnapshot.h"
#include "Common/delayedactionentity.h"

class QTimerEvent;

namespace Artworks {
    class ArtworkMetadata;
}

namespace Suggestion {
    class LocalLibraryQuery;
}

namespace Storage {
    class IDatabaseManager;
}

namespace Services {
    class ArtworksUpdateHub;
}

namespace MetadataIO {
    class MetadataIOWorker;

    class MetadataIOService:
            public QObject,
            public Common::DelayedActionEntity
    {
        Q_OBJECT
    public:
        MetadataIOService(QObject *parent = nullptr);

    public:
        void startService(Storage::IDatabaseManager &databaseManager,
                          Services::ArtworksUpdateHub &artworksUpdateHub);
        void stopService();

    public:
        void cancelBatch(quint32 batchID) const;
        bool isBusy() const;
        void waitWorkerIdle();

    public:
        void writeArtwork(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork);
        quint32 readArtworks(const Artworks::ArtworksSnapshot &snapshot) const;
        void writeArtworks(const Artworks::ArtworksSnapshot &artworks) const;
        void addArtworks(const Artworks::ArtworksSnapshot &snapshot) const;

    public:
        void searchArtworks(Suggestion::LocalLibraryQuery *query);

    signals:
        void cacheSyncRequest();

#ifdef INTEGRATION_TESTS
    public:
        MetadataIOWorker *getWorker() { return m_MetadataIOWorker; }
#endif

    private slots:
        void workerFinished();
        void onCacheSyncRequest();
        void onReadyToImportFromStorage();

        // DelayedActionEntity implementation
    protected:
        virtual void doKillTimer(int timerId) override { this->killTimer(timerId); }
        virtual int doStartTimer(int interval, Qt::TimerType timerType) override { return this->startTimer(interval, timerType); }
        virtual void doOnTimer() override;
        virtual void timerEvent(QTimerEvent *event) override { onQtTimer(event); }
        virtual void callBaseTimer(QTimerEvent *event) override { QObject::timerEvent(event); }

    private:
        MetadataIOWorker *m_MetadataIOWorker;
        bool m_IsStopped;
    };
}

#endif // METADATAIOSERVICE_H
