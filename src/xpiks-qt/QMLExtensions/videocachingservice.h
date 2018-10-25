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

#include <memory>

#include <QObject>
#include <QString>

#include "Common/messages.h"

namespace Artworks {
    class VideoArtwork;
    class ArtworksSnapshot;
}

namespace Common {
    class ISystemEnvironment;
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
            public Common::MessagesTarget<std::shared_ptr<Artworks::VideoArtwork>>
    {
        Q_OBJECT
    public:
        explicit VideoCachingService(Common::ISystemEnvironment &environment,
                                     Models::SwitcherModel &switcherModel,
                                     QObject *parent = 0);

    public:
        void startService(ImageCachingService &imageCachingService,
                          Services::ArtworksUpdateHub &updateHub,
                          MetadataIO::MetadataIOService &metadataIOService,
                          Storage::IDatabaseManager &dbManager);
        void stopService();

    public:
        virtual void handleMessage(std::shared_ptr<Artworks::VideoArtwork> const &message) override { generateThumbnail(message); }

    public:
        void generateThumbnails(const Artworks::ArtworksSnapshot &snapshot);
        void generateThumbnail(const std::shared_ptr<Artworks::VideoArtwork> &videoArtwork);
        void waitWorkerIdle();

    private:
        Common::ISystemEnvironment &m_Environment;
        Models::SwitcherModel &m_SwitcherModel;
        VideoCachingWorker *m_CachingWorker;
        volatile bool m_IsCancelled;
    };
}

#endif // VIDEOCACHINGSERVICE_H
