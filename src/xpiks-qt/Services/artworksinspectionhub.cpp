/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworksinspectionhub.h"
#include "../SpellCheck/spellcheckerservice.h"
#include "../Warnings/warningsservice.h"
#include "../Helpers/cpphelpers.h"

#define MAX_UPDATE_TIMER_DELAYS 2
#define UPDATE_TIMER_DELAY 200

namespace Services {
    ArtworksInspectionHub::ArtworksInspectionHub(SpellCheck::SpellCheckerService &spellcheckService,
                                                 Warnings::WarningsService &warningsService,
                                                 QObject *parent):
        QObject(parent),
        m_SpellCheckService(spellcheckService),
        m_WarningsService(warningsService)
    {
    }

    void ArtworksInspectionHub::checkArtworks(const Artworks::WeakArtworksSnapshot &artworks) const {
        if (artworks.empty()) { return; }
        LOG_INFO << artworks.size() << "artworks";

        const Common::WordAnalysisFlags wordAnalysisFlags = getWordAnalysisFlags();
        if (wordAnalysisFlags != Common::WordAnalysisFlags::None) {
            auto itemsToSubmit = Helpers::map(artworks, [](Artworks::ArtworkMetadata* a) { return a->getBasicModel(); });
            m_SpellCheckService.submitItems(itemsToSubmit);
        }
    }

    void ArtworksInspectionHub::checkArtworks(const Artworks::ArtworksSnapshot::Container &artworks) const {
        if (artworks.empty()) { return; }
        LOG_INFO << artworks.size() << "artworks";

        const Common::WordAnalysisFlags wordAnalysisFlags = getWordAnalysisFlags();
        if (wordAnalysisFlags != Common::WordAnalysisFlags::None) {
            auto itemsToSubmit = Helpers::map(artworks, [](const std::shared_ptr<Artworks::ArtworkMetadataLocker> &locker) {
                    return locker->getArtworkMetadata()->getBasicModel(); });
            m_SpellCheckService.submitItems(itemsToSubmit);
        }
    }

    Common::WordAnalysisFlags ArtworksInspectionHub::getWordAnalysisFlags() const {
        Common::WordAnalysisFlags result = Common::WordAnalysisFlags::None;

        if (m_SettingsModel.getUseSpellCheck()) {
            Common::SetFlag(result, Common::WordAnalysisFlags::Spelling);
        }

        if (m_SettingsModel.getDetectDuplicates()) {
            Common::SetFlag(result, Common::WordAnalysisFlags::Stemming);
        }

        return result;
    }
}
