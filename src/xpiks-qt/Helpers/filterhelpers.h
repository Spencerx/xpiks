/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef FILTERHELPERS_H
#define FILTERHELPERS_H

#include <memory>

#include <QString>

#include "Common/flags.h"

namespace Artworks {
    class ArtworkMetadata;
}

namespace Helpers {
    bool hasSearchMatch(const QString &searchTerm,
                        std::shared_ptr<Artworks::ArtworkMetadata> const &artwork,
                        Common::SearchFlags searchFlags);
}

#endif // FILTERHELPERS_H
