/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "inspectbasicmodeltemplate.h"
#include <Models/settingsmodel.h>
#include <SpellCheck/spellcheckerservice.h>

namespace Commands {
    InspectBasicModelTemplate::InspectBasicModelTemplate(SpellCheck::SpellCheckService &spellCheckService,
                                                         Models::SettingsModel &settingsModel):
        m_SpellCheckService(spellCheckService),
        m_SettingsModel(settingsModel)
    {
    }

    void InspectBasicModelTemplate::execute(const Artworks::BasicKeywordsModel *&basicModel) {
        LOG_DEBUG << "#";

        if (m_SettingsModel.getUseSpellCheck()) {
            m_SpellCheckService.submitItem(basicModel);
        }
    }
}
