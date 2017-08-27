/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "keyvaluelist.h"
#include <QImageReader>
#include <utility>
#include "artworkmetadata.h"
#include "imageartwork.h"
#include "videoartwork.h"
#include "../Helpers/filehelpers.h"

namespace Models {
    KeyValueList::KeyValueList():
        QAbstractListModel()
    {
    }

    int KeyValueList::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return getKeyValuesCount();
    }

    QVariant KeyValueList::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if ((row < 0) || (row >= getKeyValuesCount())) { return QVariant(); }

        switch (role) {
        case KeyRole: return getKey(row);
        case ValueRole: return getValue(row);
        default: return QVariant();
        }
    }

    QHash<int, QByteArray> KeyValueList::roleNames() const {
        QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
        roleNames[KeyRole] = "key";
        roleNames[ValueRole] = "value";
        return roleNames;
    }

    void ArtworkPropertiesMap::updateProperties(ArtworkMetadata *metadata) {
        Q_ASSERT(metadata != nullptr);
        Models::ImageArtwork *imageArtwork = dynamic_cast<Models::ImageArtwork*>(metadata);
        Models::VideoArtwork *videoArtwork = dynamic_cast<Models::VideoArtwork*>(metadata);

        beginResetModel();
        {
            m_ValuesHash.clear();

            if (imageArtwork != nullptr) {
                m_IsImage = true;
                setForTheImage(imageArtwork);
            } else if (videoArtwork != nullptr) {
                m_IsImage = false;
                setForTheVideo(videoArtwork);
            }
        }
        endResetModel();
    }

    void ArtworkPropertiesMap::setForTheImage(ImageArtwork *imageArtwork) {
        m_ValuesHash[int(ImageProperties::FilePathProperty)] = imageArtwork->getFilepath();
        m_ValuesHash[int(ImageProperties::FileSizeProperty)] = Helpers::describeFileSize(imageArtwork->getFileSize());

        QSize size;
        if (imageArtwork->isInitialized()) {
            size = imageArtwork->getImageSize();
        } else {
            QImageReader reader(imageArtwork->getFilepath());
            size = reader.size();
        }
        m_ValuesHash[int(ImageProperties::ImageSizeProperty)] = QString("W %1 x H %2").arg(size.width()).arg(size.height());

        m_ValuesHash[int(ImageProperties::DateTakenProperty)] = imageArtwork->getDateTaken();
        m_ValuesHash[int(ImageProperties::AttachedVectorProperty)] = imageArtwork->getAttachedVectorPath();
    }

    void ArtworkPropertiesMap::setForTheVideo(VideoArtwork *videoArtwork) {
        // TODO: FIXME:
    }

    QString ArtworkPropertiesMap::getKey(int index) const {
        static const char *imagesPropertyNames[] = {
            /*FilePathProperty*/ QT_TRANSLATE_NOOP("ImageProperties", "File path"),
            /*FileSizeProperty*/ QT_TRANSLATE_NOOP("ImageProperties", "File size"),
            /*ImageSizeProperty*/QT_TRANSLATE_NOOP("ImageProperties", "Image size"),
            /*DateTakenProperty*/QT_TRANSLATE_NOOP("ImageProperties", "Date taken"),
            /*AttachedVectorProperty*/QT_TRANSLATE_NOOP("ImageProperties", "Vector")
        };

        static const char *videoPropertyNames[] = {
            /*FilePathProperty*/ QT_TRANSLATE_NOOP("VideoProperties", "File path"),
            /*FileSizeProperty*/ QT_TRANSLATE_NOOP("VideoProperties", "File size"),
            /*CodecProperty*/ QT_TRANSLATE_NOOP("VideoProperties", "Codec"),
            /*FrameSizeProperty*/QT_TRANSLATE_NOOP("VideoProperties", "Frame size"),
            /*VideoDurationProperty*/QT_TRANSLATE_NOOP("VideoProperties", "Duration"),
            /*FPSProperty*/QT_TRANSLATE_NOOP("VideoProperties", "FPS"),
            /*AspectRatioProperty*/QT_TRANSLATE_NOOP("VideoProperties", "Aspect ratio"),
        };

        if (m_IsImage) {
            Q_ASSERT((0 <= index) && (index < int(ImageProperties::NUM_IMAGE_PROPERTIES)));
            return QObject::tr(imagesPropertyNames[index]);
        } else {
            Q_ASSERT((0 <= index) && (index < int(VideoProperties::NUM_VIDEO_PROPERTIES)));
            return QObject::tr(videoPropertyNames[index]);
        }
    }

    QString ArtworkPropertiesMap::getValue(int index) const {
        QString result;

        if (m_ValuesHash.contains(index)) {
            result = m_ValuesHash[index];
        }

        if (result.isEmpty()) {
            result = QStringLiteral("-");
        }

        return result;
    }

    int ArtworkPropertiesMap::getKeyValuesCount() const {
        if (m_IsImage) {
            return int(ImageProperties::NUM_IMAGE_PROPERTIES);
        } else {
            return int(VideoProperties::NUM_VIDEO_PROPERTIES);
        }
    }
}