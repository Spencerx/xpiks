/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VIDEOCACHINGSERVICE_H
#define VIDEOCACHINGSERVICE_H

#include <QObject>
#include <QVector>
#include <memory>
#include <vector>
#include <Common/isystemenvironment.h>
#include <Common/messages.h>
#include <Common/irefcountedobject.h>

namespace Artworks {
    class ArtworkMetadata;
    using ArtworkMetadataLocker = Common::HoldLocker<ArtworkMetadata>;
    class VideoArtwork;
    class ArtworksSnapshot;
}

namespace Storage {
    class IDatabaseManager;
}

namespace Models {
    class SwitcherModel;
}

namespace Services {
    class ArtworksUpdateHub;
}

namespace MetadataIO {
    class MetadataIOService;
}

namespace QMLExtensions {
    class VideoCachingWorker;
    class ImageCachingService;

    class VideoCachingService :
            public QObject,
            public Common::MessagesTarget<Artworks::VideoArtwork*>
    {
        Q_OBJECT
    public:
        explicit VideoCachingService(Common::ISystemEnvironment &environment,
                                     Storage::IDatabaseManager &dbManager,
                                     Models::SwitcherModel &switcherModel,
                                     QObject *parent = 0);

    public:
        void startService(ImageCachingService &imageCachingService,
                          Services::ArtworksUpdateHub &updateHub,
                          MetadataIO::MetadataIOService &metadataIOService);
        void stopService();

    public:
        virtual void handleMessage(const Artworks::VideoArtwork * &message) override { generateThumbnail(message); }

    public:
        void generateThumbnails(const Artworks::ArtworksSnapshot &snapshot);
        void generateThumbnail(Artworks::VideoArtwork *videoArtwork);
        void waitWorkerIdle();

    private:
        Common::ISystemEnvironment &m_Environment;
        Storage::IDatabaseManager &m_DatabaseManager;
        Models::SwitcherModel &m_SwitcherModel;
        VideoCachingWorker *m_CachingWorker;
        volatile bool m_IsCancelled;
    };
}

#endif // VIDEOCACHINGSERVICE_H
