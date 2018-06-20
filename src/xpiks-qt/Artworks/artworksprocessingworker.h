/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKSPROCESSINGWORKER_H
#define ARTWORKSPROCESSINGWORKER_H

#include <functional>
#include "../Common/itemprocessingworker.h"
#include "artworkssnapshot.h"
#include "../Helpers/cpphelpers.h"

namespace Artworks {
    template<typename ItemType, typename ResultType=void>
    class ArtworkProcessingWorker: public Common::ItemProcessingWorker<ItemType, ResultType>
    {
    public:
        ArtworkProcessingWorker(int batchSize):
            Common::ItemProcessingWorker(batchSize)
        { }

    public:
        batch_id_t submitArtworks(const WeakArtworksSnapshot &snapshot) {
            std::function<std::shared_ptr<ItemType>(const ArtworkMetadata *)> mapper =
                    std::bind(&ArtworkProcessingWorker::mapToWorkItem, this, std::placeholders::_1);
            auto items = Helpers::map(snapshot, mapper);
            return this->submitItems(items);
        }

        batch_id_t submitArtwork(const ArtworkMetadata *artwork) {
            auto item = mapToWorkItem(artwork);
            return this->submitItem(item);
        }

    protected:
        virtual std::shared_ptr<ItemType> mapToWorkItem(const ArtworkMetadata *artwork) = 0;
    };
}

#endif // ARTWORKSPROCESSINGWORKER_H
