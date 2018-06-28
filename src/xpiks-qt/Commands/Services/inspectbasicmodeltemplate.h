/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INSPECTBASICMODELTEMPLATE_H
#define INSPECTBASICMODELTEMPLATE_H

#include <Commands/Base/icommandtemplate.h>

namespace SpellCheck {
    class SpellCheckerService;
}

namespace Warnings {
    class WarningsService;
}

namespace Models {
    class SettingsModel;
}

namespace Artworks {
    class BasicKeywordsModel;
}

namespace Commands {
    class InspectBasicModelTemplate: public ICommandTemplate<Artworks::BasicKeywordsModel*>
    {
    public:
        InspectBasicModelTemplate(SpellCheck::SpellCheckerService &spellCheckService,
                                  Models::SettingsModel &settingsModel);

    public:
        virtual void execute(const Artworks::BasicKeywordsModel *&basicModel) override;
        virtual void undo(const Artworks::BasicKeywordsModel *&basicModel) override { execute(basicModel); }

    private:
        SpellCheck::SpellCheckerService &m_SpellCheckService;
        Models::SettingsModel &m_SettingsModel;
    };
}

#endif // INSPECTBASICMODELTEMPLATE_H
