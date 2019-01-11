/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef KEYVALUELIST_H
#define KEYVALUELIST_H

#include <memory>

#include <QAbstractListModel>
#include <QHash>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>

class QByteArray;
class QModelIndex;

namespace Artworks {
    class ArtworkMetadata;
    class ImageArtwork;
    class VideoArtwork;
}

namespace Models {

    class KeyValueList: public QAbstractListModel
    {
        Q_OBJECT
    public:
        KeyValueList();

    public:
        enum KeyValueList_Roles {
            KeyRole = Qt::UserRole + 1,
            ValueRole
        };

        // QAbstractItemModel interface
    public:
        virtual int rowCount(const QModelIndex &parent) const override;
        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QHash<int, QByteArray> roleNames() const override;

    protected:
        virtual QString getKey(int index) const = 0;
        virtual QString getValue(int index) const = 0;
        virtual int getKeyValuesCount() const = 0;
    };

    class ArtworkPropertiesMap: public KeyValueList {
        Q_OBJECT
    public:
        ArtworkPropertiesMap(): m_IsImage(false) {}

    public:
        void updateProperties(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork);

    private:
        void setForTheImage(std::shared_ptr<Artworks::ImageArtwork> const &imageArtwork);
        void setForTheVideo(std::shared_ptr<Artworks::VideoArtwork> const &videoArtwork);

    private:
        enum class ImageProperties {
            FilePathProperty,
            FileSizeProperty,
            FileAccessProperty,
            ImageSizeProperty,
            DateTakenProperty,
            AttachedVectorProperty,
            NUM_IMAGE_PROPERTIES
        };

        enum class VideoProperties {
            FilePathProperty,
            FileSizeProperty,
            FileAccessProperty,
            CodecProperty,
            FrameSizeProperty,
            VideoDurationProperty,
            FrameRateProperty,
            BitRateProperty,
            NUM_VIDEO_PROPERTIES
        };

        // KeyValueList interface
    protected:
        virtual QString getKey(int index) const override;
        virtual QString getValue(int index) const override;
        virtual int getKeyValuesCount() const override;

    private:
        QHash<int, QString> m_ValuesHash;
        bool m_IsImage;
    };
}

#endif // KEYVALUELIST_H
