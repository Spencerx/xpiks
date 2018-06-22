/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMPOSITECOMMANDTEMPLATE_H
#define COMPOSITECOMMANDTEMPLATE_H

#include <memory>
#include <vector>
#include <initializer_list>
#include "iartworkscommandtemplate.h"

namespace Commands {
    class CompositeCommandTemplate: IArtworksCommandTemplate
    {
    public:
        CompositeCommandTemplate(std::initializer_list<std::shared_ptr<IArtworksCommandTemplate> > list);

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(Artworks::ArtworksSnapshot &snapshot) override;
        virtual void undo(Artworks::ArtworksSnapshot &snapshot) override;

    private:
        std::vector<std::shared_ptr<IArtworksCommandTemplate> > m_Templates;
    };
}

#endif // COMPOSITECOMMANDTEMPLATE_H
