/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef METADATADUPLICATES_H
#define METADATADUPLICATES_H

#include "imetadataduplicates.h"
#include <memory>

namespace Artworks {
    class ArtworkMetadata;
    class BasicMetadataModel;
}

namespace SpellCheck {
    class BasicModelDuplicates: public IMetadataDuplicates {
    public:
        BasicModelDuplicates(Artworks::BasicMetadataModel &basicModel);

        // IMetadataDuplicates interface
    public:
        virtual Artworks::BasicMetadataModel &getBasicModel() override { return m_BasicModel; }
        virtual QString getDescriptionDuplicates() override;
        virtual QString getTitleDuplicates() override;
        virtual QString getKeywordsDuplicates() override;
        virtual bool hasThumbnail() override { return false; }

    private:
        Artworks::BasicMetadataModel &m_BasicModel;
    };

    class ArtworkMetadataDuplicates: public BasicModelDuplicates {
    public:
        ArtworkMetadataDuplicates(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork);

    public:
        std::shared_ptr<Artworks::ArtworkMetadata> const &getArtwork() const { return m_Artwork; }
        virtual bool hasThumbnail() override { return true; }

    private:
        std::shared_ptr<Artworks::ArtworkMetadata> m_Artwork;
    };
}

#endif // METADATADUPLICATES_H
