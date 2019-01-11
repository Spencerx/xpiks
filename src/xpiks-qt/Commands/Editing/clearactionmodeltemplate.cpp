/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "clearactionmodeltemplate.h"

#include <QtDebug>

#include "Common/logging.h"
#include "Models/iactionmodel.h"

namespace Commands {
    ClearActionModelTemplate::ClearActionModelTemplate(Models::IActionModel &model):
        m_Model(model)
    {
    }

    void ClearActionModelTemplate::execute(const Artworks::ArtworksSnapshot &) {
        LOG_DEBUG << "#";
        m_Model.resetModel();
    }

    ClearActionModelCommand::ClearActionModelCommand(Models::IActionModel &model):
        m_Model(model)
    {
    }

    void ClearActionModelCommand::execute() {
        LOG_DEBUG << "#";
        m_Model.resetModel();
    }
}
