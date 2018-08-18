/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKSINSPECTIONHUB_H
#define ARTWORKSINSPECTIONHUB_H

#include <Common/messages.h>
#include <Common/types.h>

namespace SpellCheck {
    class SpellCheckService;
}

namespace Warnings {
    class WarningsService;
}

namespace Models {
    class SettingsModel;
}

namespace Artworks {
    class ArtworksSnapshot;
    class ArtworkMetadata;
    class BasicKeywordsModel;
}

namespace Services {
    using ArtworkInspectionType = Common::NamedType<Artworks::ArtworkMetadata*, Common::MessageType::SpellCheck>;
    using BasicModelInspectionType = Common::NamedType<Artworks::BasicKeywordsModel*, Common::MessageType::SpellCheck>;
    using ArtworksArrayInspectionType = Common::NamedType<std::vector<Artworks::ArtworkMetadata*>, Common::MessageType::SpellCheck>;

    class ArtworksInspectionHub:
            public Common::MessagesTarget<ArtworkInspectionType>,
            public Common::MessagesTarget<BasicModelInspectionType>,
            public Common::MessagesTarget<ArtworksArrayInspectionType>
    {
        using Common::MessagesTarget<ArtworkInspectionType>::handleMessage;
        using Common::MessagesTarget<BasicModelInspectionType>::handleMessage;
        using Common::MessagesTarget<ArtworksArrayInspectionType>::handleMessage;

    public:
        ArtworksInspectionHub(SpellCheck::SpellCheckService &spellCheckService,
                              Warnings::WarningsService &warningsService,
                              Models::SettingsModel &settingsModel);

    public:
        virtual void handleMessage(ArtworkInspectionType const &change) override;
        virtual void handleMessage(BasicModelInspectionType const &change) override;
        virtual void handleMessage(ArtworksArrayInspectionType const &change) override;

    public:
        void inspectArtwork(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork);
        void inspectArtworks(Artworks::ArtworksSnapshot const &snapshot);
        void inspectBasicModel(Artworks::BasicKeywordsModel const &basicModel);

    private:
        bool isSpellCheckAvailable() const;

    private:
        SpellCheck::SpellCheckService &m_SpellCheckService;
        Warnings::WarningsService &m_WarningsService;
        Models::SettingsModel &m_SettingsModel;
    };
}

#endif // ARTWORKSINSPECTIONHUB_H
