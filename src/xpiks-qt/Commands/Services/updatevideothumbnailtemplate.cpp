/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "updatevideothumbnailtemplate.h"
#include <Artworks/videoartwork.h>
#include <Artworks/artworkssnapshot.h>
#include <QMLExtensions/videocachingservice.h>
#include <Services/artworksupdatehub.h>
#include <Models/Artworks/artworkslistmodel.h>

namespace Commands {
    UpdateVideoThumbnailTemplate::UpdateVideoThumbnailTemplate(QMLExtensions::VideoCachingService &videoCachingService,
                                                               Services::ArtworksUpdateHub &artworksUpdateHub):
        m_VideoCachingService(videoCachingService),
        m_ArtworksUpdateHub(artworksUpdateHub)
    {
    }

    void UpdateVideoThumbnailTemplate::execute(const Artworks::ArtworksSnapshot &snapshot) {
        const size_t size = snapshot.size();

        for (size_t i = 0; i < size; i++) {
            Artworks::ArtworkMetadata *artwork = snapshot.get(i);
            Artworks::VideoArtwork *videoArtwork = dynamic_cast<Artworks::VideoArtwork*>(artwork);
            if (videoArtwork == nullptr) { continue; }

            if (!videoArtwork->isThumbnailGenerated()) {
                m_VideoCachingService.generateThumbnail(videoArtwork);
            } else {
                m_ArtworksUpdateHub.updateArtwork(videoArtwork->getItemID(),
                                                  videoArtwork->getLastKnownIndex(),
                                                  QSet<int>() << Models::ArtworksListModel::ArtworkThumbnailRole);
            }
        }
    }
}
