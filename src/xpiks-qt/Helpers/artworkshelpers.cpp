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
#include <Artworks/artworkmetadata.h>
#include <Artworks/imageartwork.h>
#include <Artworks/videoartwork.h>
#include <Artworks/artworkssnapshot.h>
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

    void splitImagesVideo(const Artworks::ArtworksSnapshot &rawSnapshot,
                          Artworks::ArtworksSnapshot &imagesRawSnapshot,
                          Artworks::ArtworksSnapshot &videoRawSnapshot) {
        const size_t size = rawSnapshot.size();
        imagesRawSnapshot.reserve(size / 2);
        videoRawSnapshot.reserve(size / 2);

        for (auto &artwork: rawSnapshot) {
            Q_ASSERT(artwork != nullptr);
            if (artwork == nullptr) { continue; }

            auto imageArtwork = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
            if (imageArtwork != nullptr) {
                imagesRawSnapshot.append(artwork);
            } else {
                auto videoArtwork = std::dynamic_pointer_cast<Artworks::VideoArtwork>(artwork);
                if (videoArtwork != nullptr) {
                    videoRawSnapshot.append(artwork);
                } else {
                    Q_ASSERT(false);
                    LOG_WARNING << "Unknown type";
                }
            }
        }
    }

    int retrieveImagesCount(const Artworks::ArtworksSnapshot &rawSnapshot) {
        int count = 0;

        for (auto &artwork: rawSnapshot) {
            Q_ASSERT(artwork != nullptr);
            if (artwork == nullptr) { continue; }

            auto imageArtwork = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
            if (imageArtwork != nullptr) {
                count++;
            }
        }

        return count;
    }

    int retrieveVideosCount(const Artworks::ArtworksSnapshot &rawSnapshot) {
        int count = 0;

        for (auto &artwork: rawSnapshot) {
            Q_ASSERT(artwork != nullptr);
            if (artwork == nullptr) { continue; }

            auto videoArtwork = std::dynamic_pointer_cast<Artworks::VideoArtwork>(artwork);
            if (videoArtwork != nullptr) {
                count++;
            }
        }

        return count;
    }

    int findAndAttachVectors(const Artworks::ArtworksSnapshot &snapshot, QVector<int> &modifiedIndices) {
        LOG_DEBUG << "#";
        int attachedCount = 0;
        const size_t size = snapshot.size();
        modifiedIndices.reserve((int)size);

        for (size_t i = 0; i < size; ++i) {
            auto &artwork = snapshot.get(i);
            auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);

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
