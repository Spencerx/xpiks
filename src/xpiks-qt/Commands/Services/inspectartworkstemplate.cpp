/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "inspectartworkstemplate.h"
#include <SpellCheck/spellcheckerservice.h>
#include <Artworks/artworkssnapshot.h>
#include <Helpers/cpphelpers.h>
#include <Models/settingsmodel.h>
#include <Warnings/warningsservice.h>

namespace Commands {
    InspectArtworksTemplate::InspectArtworksTemplate(SpellCheck::SpellCheckerService &spellCheckService,
                                                     Warnings::WarningsService &warningsService,
                                                     Models::SettingsModel &settingsModel):
        m_SpellCheckService(spellCheckService),
        m_WarningsService(warningsService),
        m_SettingsModel(settingsModel)
    {
    }

    void InspectArtworksTemplate::execute(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "#";

        if (m_SettingsModel.getUseSpellCheck()) {
            m_SpellCheckService.submitItems(snapshot.getWeakSnapshot());
        } else {
            m_WarningsService.submitItems(snapshot.getWeakSnapshot());
        }
    }
}
