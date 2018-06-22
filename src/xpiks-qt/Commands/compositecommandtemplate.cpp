/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "compositecommandtemplate.h"

namespace Commands {
    CompositeCommandTemplate::CompositeCommandTemplate(
            std::initializer_list<std::shared_ptr<IArtworksCommandTemplate> > list) {
        m_Templates.insert(m_Templates.end(), list.begin(), list.end());
    }

    void CompositeCommandTemplate::execute(Artworks::ArtworksSnapshot &snapshot) {
        for (auto &t: m_Templates) {
            t->execute(snapshot);
        }
    }

    void CompositeCommandTemplate::undo(Artworks::ArtworksSnapshot &snapshot) {
        for (auto &t: m_Templates) {
            t->undo(snapshot);
        }
    }
}
