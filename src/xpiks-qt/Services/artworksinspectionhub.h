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
    class ArtworksInspectionHub:
            public Common::MessagesTarget<Artworks::ArtworkMetadata*>,
            public Common::MessagesTarget<Artworks::BasicKeywordsModel*>,
            public Common::MessagesTarget<std::vector<Artworks::ArtworkMetadata*>>
    {
        using Common::MessagesTarget<Artworks::ArtworkMetadata*>::handleMessage;
        using Common::MessagesTarget<Artworks::BasicKeywordsModel*>::handleMessage;
        using Common::MessagesTarget<std::vector<Artworks::ArtworkMetadata*>>::handleMessage;

    public:
        ArtworksInspectionHub(SpellCheck::SpellCheckService &spellCheckService,
                              Warnings::WarningsService &warningsService,
                              Models::SettingsModel &settingsModel);

    public:
        virtual void handleMessage(Artworks::ArtworkMetadata* const &change) override;
        virtual void handleMessage(Artworks::BasicKeywordsModel* const &change) override;
        virtual void handleMessage(std::vector<Artworks::ArtworkMetadata*> const &change) override;

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
