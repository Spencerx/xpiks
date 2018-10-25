/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef PROXYINDEXMIDDLWARE_H
#define PROXYINDEXMIDDLWARE_H

#include <memory>

#include "QMLExtensions/iuicommandmiddleware.h"

namespace Models {
    class FilteredArtworksListModel;
}

namespace QMLExtensions {
    struct UIAction;

    class ProxyIndexMiddlware: public IUICommandMiddlware
    {
    public:
        ProxyIndexMiddlware(Models::FilteredArtworksListModel &filteredArtworksList);

        // IUICommandMiddlware interface
    public:
        virtual std::shared_ptr<UIAction> process(const std::shared_ptr<UIAction> &action) override;

    private:
        Models::FilteredArtworksListModel &m_FilteredArtworksList;
    };
}

#endif // PROXYINDEXMIDDLWARE_H
