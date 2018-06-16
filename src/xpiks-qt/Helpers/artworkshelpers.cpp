/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworkshelpers.h"
#include "../Models/artworkmetadata.h"
#include "../Models/imageartwork.h"
#include "../Models/videoartwork.h"
#include "filehelpers.h"

namespace Helpers {
    void splitImagesVideo(const QVector<Artworks::ArtworkMetadata *> &artworks, QVector<Artworks::ArtworkMetadata *> &imageArtworks, QVector<Artworks::ArtworkMetadata *> &videoArtworks) {
        int size = artworks.size();
        imageArtworks.reserve(size / 2);
        videoArtworks.reserve(size / 2);

        for (auto *artwork: artworks) {
            Q_ASSERT(artwork != nullptr);
            if (artwork == nullptr) { continue; }

            Artworks::ImageArtwork *imageArtwork = dynamic_cast<Artworks::ImageArtwork*>(artwork);
            if (imageArtwork != nullptr) {
                imageArtworks.append(artwork);
            } else {
                Artworks::VideoArtwork *videoArtwork = dynamic_cast<Artworks::VideoArtwork*>(artwork);
                if (videoArtwork != nullptr) {
                    videoArtworks.append(artwork);
                } else {
                    Q_ASSERT(false);
                    LOG_WARNING << "Unknown type";
                }
            }
        }
    }

    void splitImagesVideo(const std::vector<std::shared_ptr<Artworks::ArtworkMetadataLocker> > &rawSnapshot,
                          std::vector<std::shared_ptr<Artworks::ArtworkMetadataLocker> > &imagesRawSnapshot,
                          std::vector<std::shared_ptr<Artworks::ArtworkMetadataLocker> > &videoRawSnapshot) {
        const size_t size = rawSnapshot.size();
        imagesRawSnapshot.reserve(size / 2);
        videoRawSnapshot.reserve(size / 2);

        for (auto &locker: rawSnapshot) {
            Artworks::ArtworkMetadata *artwork = locker->getArtworkMetadata();

            Q_ASSERT(artwork != nullptr);
            if (artwork == nullptr) { continue; }

            Artworks::ImageArtwork *imageArtwork = dynamic_cast<Artworks::ImageArtwork*>(artwork);
            if (imageArtwork != nullptr) {
                imagesRawSnapshot.push_back(locker);
            } else {
                Artworks::VideoArtwork *videoArtwork = dynamic_cast<Artworks::VideoArtwork*>(artwork);
                if (videoArtwork != nullptr) {
                    videoRawSnapshot.push_back(locker);
                } else {
                    Q_ASSERT(false);
                    LOG_WARNING << "Unknown type";
                }
            }
        }
    }

    int retrieveImagesCount(const std::vector<std::shared_ptr<Artworks::ArtworkMetadataLocker> > &rawSnapshot) {
        int count = 0;

        for (auto &locker: rawSnapshot) {
            Artworks::ArtworkMetadata *artwork = locker->getArtworkMetadata();

            Q_ASSERT(artwork != nullptr);
            if (artwork == nullptr) { continue; }

            Artworks::ImageArtwork *imageArtwork = dynamic_cast<Artworks::ImageArtwork*>(artwork);
            if (imageArtwork != nullptr) {
                count++;
            }
        }

        return count;
    }

    int retrieveVideosCount(const std::vector<std::shared_ptr<Artworks::ArtworkMetadataLocker> > &rawSnapshot) {
        int count = 0;

        for (auto &locker: rawSnapshot) {
            Artworks::ArtworkMetadata *artwork = locker->getArtworkMetadata();

            Q_ASSERT(artwork != nullptr);
            if (artwork == nullptr) { continue; }

            Artworks::VideoArtwork *imageArtwork = dynamic_cast<Artworks::VideoArtwork*>(artwork);
            if (imageArtwork != nullptr) {
                count++;
            }
        }

        return count;
    }

    int findAndAttachVectors(const Artworks::WeakArtworksSnapshot &artworksList, QVector<int> &modifiedIndices) {
        LOG_DEBUG << "#";
        int attachedCount = 0;
        const size_t size = artworksList.size();
        modifiedIndices.reserve((int)size);

        for (size_t i = 0; i < size; ++i) {
            Artworks::ArtworkMetadata *artwork = artworksList.at(i);
            Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork *>(artwork);

            if (image == NULL) { continue; }

            if (image->hasVectorAttached()) {
                attachedCount++;
                modifiedIndices.append((int)i);
                continue;
            }

            const QString &filepath = image->getFilepath();
            QStringList vectors = Helpers::convertToVectorFilenames(filepath);

            foreach (const QString &item, vectors) {
                if (QFileInfo(item).exists()) {
                    image->attachVector(item);
                    attachedCount++;
                    modifiedIndices.append((int)i);
                    break;
                }
            }
        }

        return attachedCount;
    }
}
