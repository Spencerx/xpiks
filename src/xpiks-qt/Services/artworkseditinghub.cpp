/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworkseditinghub.h"
#include <Services/Warnings/warningsservice.h>
#include <Services/SpellCheck/ispellcheckservice.h>
#include <MetadataIO/metadataioservice.h>
#include <Models/settingsmodel.h>
#include <Artworks/artworkmetadata.h>
#include <Artworks/artworkssnapshot.h>
#include <Helpers/cpphelpers.h>

namespace Services {
    ArtworksEditingHub::ArtworksEditingHub(SpellCheck::ISpellCheckService &spellCheckService,
                                           Warnings::WarningsService &warningsService,
                                           MetadataIO::MetadataIOService &metadataIOService,
                                           Models::SettingsModel &settingsModel):
        m_SpellCheckService(spellCheckService),
        m_WarningsService(warningsService),
        m_MetadataIOService(metadataIOService),
        m_SettingsModel(settingsModel)
    {
    }

    void ArtworksEditingHub::handleMessage(const ItemInspectionType &change) {
        inspectItem(change.get());
    }

    void ArtworksEditingHub::handleMessage(const ItemArrayInspectionType &change) {
        inspectItems(change.get());
    }

    void ArtworksEditingHub::handleMessage(const ArtworkeEditingType &change) {
        inspectItem(change.get());
        m_MetadataIOService.writeArtwork(change.get());
    }

    void ArtworksEditingHub::inspectItem(const std::shared_ptr<Artworks::IBasicModelSource> &item) {
        LOG_DEBUG << "#";
        const bool spellCheckAvailable = isSpellCheckAvailable();
        if (spellCheckAvailable) {
            m_SpellCheckService.submitItem(item, Common::SpellCheckFlags::All);
        }

        if (!spellCheckAvailable || item->getBasicModel().isEmpty()) {
            auto artwork = std::dynamic_pointer_cast<Artworks::ArtworkMetadata>(item);
            if (artwork != nullptr) {
                m_WarningsService.submitItem(artwork);
            }
        }
    }

    void ArtworksEditingHub::inspectItems(const std::vector<std::shared_ptr<Artworks::IBasicModelSource> > &items) {
        LOG_INFO << items.size() << "items";
        const bool spellCheckAvailable = isSpellCheckAvailable();
        if (spellCheckAvailable) {
            m_SpellCheckService.submitItems(items, Common::SpellCheckFlags::All);
        }

        if (!items.empty() &&
                (std::dynamic_pointer_cast<Artworks::ArtworkMetadata>(items.front()) != nullptr)) {
            using ArtworkPtr = std::shared_ptr<Artworks::ArtworkMetadata>;
            using SourcePtr = std::shared_ptr<Artworks::IBasicModelSource>;
            m_WarningsService.submitItems(
                        Helpers::filterMap<SourcePtr, ArtworkPtr>(
                            items,
                            [&spellCheckAvailable](const SourcePtr &src) {
                return (!spellCheckAvailable) || (src->getBasicModel().isEmpty());
            },
            [](const SourcePtr &src) {
                return std::dynamic_pointer_cast<Artworks::ArtworkMetadata>(src);
            }));
        }
    }

    void ArtworksEditingHub::inspectItems(const Artworks::ArtworksSnapshot &snapshot) {
        const bool spellCheckAvailable = isSpellCheckAvailable();

        if (spellCheckAvailable) {
            using ArtworkPtr = std::shared_ptr<Artworks::ArtworkMetadata>;
            using SourcePtr = std::shared_ptr<Artworks::IBasicModelSource>;
            m_SpellCheckService.submitItems(
                        Helpers::map<ArtworkPtr, SourcePtr>(snapshot.getRawData(),
                                                            [](const ArtworkPtr &src) {
                return std::dynamic_pointer_cast<Artworks::IBasicModelSource>(src);
            }),
                        Common::SpellCheckFlags::All);
        }

        {
            m_WarningsService.submitItems(snapshot);
        }
    }

    bool ArtworksEditingHub::isSpellCheckAvailable() const {
        return m_SettingsModel.getUseSpellCheck();
    }
}
