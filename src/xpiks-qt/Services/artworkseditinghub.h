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

#include <memory>
#include <Common/messages.h>
#include <Common/types.h>

namespace SpellCheck {
    class ISpellCheckService;
}

namespace Warnings {
    class WarningsService;
}

namespace Models {
    class SettingsModel;
}

namespace MetadataIO {
    class MetadataIOService;
}

namespace Artworks {
    class IBasicModelSource;
    class ArtworksSnapshot;
    class ArtworkMetadata;
}

namespace Services {
    using ArtworkeEditingType = Common::NamedType<std::shared_ptr<Artworks::ArtworkMetadata>, Common::MessageType::EditingPaused>;
    using ItemInspectionType = Common::NamedType<std::shared_ptr<Artworks::IBasicModelSource>, Common::MessageType::SpellCheck>;
    using ItemArrayInspectionType = Common::NamedType<std::vector<std::shared_ptr<Artworks::IBasicModelSource>>, Common::MessageType::SpellCheck>;

    class ArtworksEditingHub:
            public Common::MessagesTarget<ItemInspectionType>,
            public Common::MessagesTarget<ItemArrayInspectionType>,
            public Common::MessagesTarget<ArtworkeEditingType>
    {
        using Common::MessagesTarget<ItemInspectionType>::handleMessage;
        using Common::MessagesTarget<ItemArrayInspectionType>::handleMessage;
        using Common::MessagesTarget<ArtworkeEditingType>::handleMessage;

    public:
        ArtworksEditingHub(SpellCheck::ISpellCheckService &spellCheckService,
                           Warnings::WarningsService &warningsService,
                           MetadataIO::MetadataIOService &metadataIOService,
                           Models::SettingsModel &settingsModel);

    public:
        virtual void handleMessage(ItemInspectionType const &change) override;
        virtual void handleMessage(ItemArrayInspectionType const &change) override;
        virtual void handleMessage(ArtworkeEditingType const &change) override;

    public:
        void inspectItem(std::shared_ptr<Artworks::IBasicModelSource> const &item);
        void inspectItems(std::vector<std::shared_ptr<Artworks::IBasicModelSource>> const &items);
        void inspectItems(Artworks::ArtworksSnapshot const &snapshot);

    private:
        bool isSpellCheckAvailable() const;

    private:
        SpellCheck::ISpellCheckService &m_SpellCheckService;
        Warnings::WarningsService &m_WarningsService;
        MetadataIO::MetadataIOService &m_MetadataIOService;
        Models::SettingsModel &m_SettingsModel;
    };
}

#endif // ARTWORKSINSPECTIONHUB_H
