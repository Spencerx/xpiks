/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "proxyindexmiddlware.h"
#include <Models/Artworks/filteredartworkslistmodel.h>
#include "uicommandid.h"
#include <Helpers/uihelpers.h>

namespace QMLExtensions {
    ProxyIndexMiddlware::ProxyIndexMiddlware(Models::FilteredArtworksListModel &filteredArtworksList):
        m_FilteredArtworksList(filteredArtworksList)
    {
    }

    std::shared_ptr<UIAction> ProxyIndexMiddlware::process(const std::shared_ptr<UIAction> &action) {
        if (action->m_CommandID != UICommandID::SetupArtworkEdit) { return action; }

        int originalIndex = Helpers::convertToInt(action->m_Value, -1);
        int proxyIndex = m_FilteredArtworksList.getProxyIndex(originalIndex);
        return std::make_shared<UIAction>(UICommandID::SetupProxyArtworkEdit, QVariant::fromValue(proxyIndex));
    }
}
