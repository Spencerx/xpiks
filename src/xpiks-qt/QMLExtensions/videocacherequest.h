/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VIDEOCACHEREQUEST_H
#define VIDEOCACHEREQUEST_H

#include <QString>
#include "../Models/videoartwork.h"
#include "../Common/flags.h"

namespace libthmbnlr {
    struct VideoFileMetadata;
}

namespace QMLExtensions {
    class VideoCacheRequest {
    private:
        enum RequestFlags {
            QuickThumbnailFlag = 1 << 0,
            GoodQualityAllowed = 1 << 1,
            RecacheFlag = 1 << 2,
            WithDelayFlag = 1 << 3,
            RepeatRequestFlag = 1 << 4
        };

    public:
        VideoCacheRequest(Models::VideoArtwork *videoArtwork, bool recache, bool quickThumbnail=true, bool withDelay=false, bool allowGoodQuality=false):
            m_VideoArtwork(videoArtwork),
            m_Flags(0)
        {
            if (m_VideoArtwork != nullptr) {
                m_VideoArtwork->acquire();
            }

            Common::ApplyFlag(m_Flags, recache, RecacheFlag);
            Common::ApplyFlag(m_Flags, quickThumbnail, QuickThumbnailFlag);
            Common::ApplyFlag(m_Flags, withDelay, WithDelayFlag);
            Common::ApplyFlag(m_Flags, allowGoodQuality, GoodQualityAllowed);
        }

        virtual ~VideoCacheRequest() {
            if (m_VideoArtwork != NULL) {
                m_VideoArtwork->release();
            }
        }

    public:
        const QString &getFilepath() const { return m_VideoArtwork->getFilepath(); }
        bool getNeedRecache() const { return Common::HasFlag(m_Flags, RecacheFlag); }
        bool getWithDelay() const { return Common::HasFlag(m_Flags, WithDelayFlag); }
        bool getIsQuickThumbnail() const { return Common::HasFlag(m_Flags, QuickThumbnailFlag); }
        bool getGoodQualityAllowed() const { return Common::HasFlag(m_Flags, GoodQualityAllowed); }
        Common::ID_t getArtworkID() const { return m_VideoArtwork->getItemID(); }
        size_t getLastKnownIndex() const { return m_VideoArtwork->getLastKnownIndex(); }
        const QString &getThumbnailPath() const { return m_VideoArtwork->getThumbnailPath(); }
        bool isRepeated() const { return Common::HasFlag(m_Flags, RepeatRequestFlag); }

    public:
        void setRepeatRequest() { Common::ApplyFlag(m_Flags, true, RepeatRequestFlag); }
        void setGoodQualityRequest() { Common::ApplyFlag(m_Flags, false, QuickThumbnailFlag); }
        void setThumbnailPath(const QString &path) { m_VideoArtwork->setThumbnailPath(path); }
        void setVideoMetadata(const libthmbnlr::VideoFileMetadata &metadata) { m_VideoArtwork->setVideoMetadata(metadata); }
        Models::VideoArtwork *getArtwork() { return m_VideoArtwork; }

    private:
        Models::VideoArtwork *m_VideoArtwork;
        Common::flag_t m_Flags;
    };
}

#endif // VIDEOCACHEREQUEST_H
