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
    using ArtworkUpdateType = Common::NamedType<Artworks::ArtworkMetadata*, Common::MessageType::SpellCheck>;
    using BasicModelUpdateType = Common::NamedType<Artworks::BasicKeywordsModel*, Common::MessageType::SpellCheck>;
    using ArtworksArrayUpdateType = Common::NamedType<std::vector<Artworks::ArtworkMetadata*>, Common::MessageType::SpellCheck>;

    class ArtworksInspectionHub:
            public Common::MessagesTarget<ArtworkUpdateType>,
            public Common::MessagesTarget<BasicModelUpdateType>,
            public Common::MessagesTarget<ArtworksArrayUpdateType>
    {
        using Common::MessagesTarget<ArtworkUpdateType>::handleMessage;
        using Common::MessagesTarget<BasicModelUpdateType>::handleMessage;
        using Common::MessagesTarget<ArtworksArrayUpdateType>::handleMessage;

    public:
        ArtworksInspectionHub(SpellCheck::SpellCheckService &spellCheckService,
                              Warnings::WarningsService &warningsService,
                              Models::SettingsModel &settingsModel);

    public:
        virtual void handleMessage(ArtworkUpdateType const &change) override;
        virtual void handleMessage(BasicModelUpdateType const &change) override;
        virtual void handleMessage(ArtworksArrayUpdateType const &change) override;

    public:
        void inspectArtwork(Artworks::ArtworkMetadata *artwork);
        void inspectArtworks(Artworks::ArtworksSnapshot const &snapshot);
        void inspectBasicModel(Artworks::BasicKeywordsModel *basicModel);

    private:
        bool isSpellCheckAvailable() const;

    private:
        SpellCheck::SpellCheckService &m_SpellCheckService;
        Warnings::WarningsService &m_WarningsService;
        Models::SettingsModel &m_SettingsModel;
    };
}

#endif // ARTWORKSINSPECTIONHUB_H
