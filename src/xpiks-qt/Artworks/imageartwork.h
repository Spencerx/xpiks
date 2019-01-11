/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IMAGEARTWORK_H
#define IMAGEARTWORK_H

#include <QDateTime>
#include <QObject>
#include <QSize>
#include <QString>
#include <QtGlobal>

#include "Common/flags.h"
#include "Common/types.h"
#include "artworkmetadata.h"

namespace MetadataIO {
    struct CachedArtwork;
    struct OriginalMetadata;
}

namespace Artworks {
    class ImageArtwork: public ArtworkMetadata
    {
        Q_OBJECT
    public:
        ImageArtwork(const QString &filepath, Common::ID_t ID, qint64 directoryID);

    private:
        enum ImageArtworkFlags {
            FlagHasVectorAttached = 1 << 0
        };

        inline bool getHasVectorAttachedFlag() const { return Common::HasFlag(m_ImageFlags, FlagHasVectorAttached); }
        inline void setHasVectorAttachedFlag(bool value) { Common::ApplyFlag(m_ImageFlags, value, FlagHasVectorAttached); }

    public:
        QSize getImageSize() const { return m_ImageSize; }
        void setImageSize(const QSize &size) { m_ImageSize = size; }
        void setDateTimeOriginal(const QDateTime &dateTime) { m_DateTimeOriginal = dateTime; }
        const QString &getAttachedVectorPath() const { return m_AttachedVector; }
        QString getDateTaken() const { return m_DateTimeOriginal.toString(); }
        const QDateTime &getDateTimeOriginal() const { return m_DateTimeOriginal; }
        bool hasVectorAttached() const { return getHasVectorAttachedFlag(); }

    public:
        void attachVector(const QString &vectorFilepath);
        void detachVector();

    protected:
        virtual bool initFromOriginUnsafe(const MetadataIO::OriginalMetadata &originalMetadata) override;
        virtual bool initFromStorageUnsafe(const MetadataIO::CachedArtwork &cachedArtwork) override;

    private:
        QSize m_ImageSize;
        QString m_AttachedVector;
        QDateTime m_DateTimeOriginal;
        volatile Common::flag_t m_ImageFlags;
    };
}

#endif // IMAGEARTWORK_H
