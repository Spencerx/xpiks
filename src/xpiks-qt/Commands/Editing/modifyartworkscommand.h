/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMBINEDEDITCOMMAND_H
#define COMBINEDEDITCOMMAND_H

#include <cstddef>
#include <memory>
#include <utility>

#include <QObject>
#include <QString>

#include "Artworks/artworkssnapshot.h"
#include "Commands/Base/templatedcommand.h"

namespace Artworks {
    class ArtworkMetadata;
}

namespace Commands {
    template <typename T> class ICommandTemplate;

    class ModifyArtworksCommand: public TemplatedCommand<Artworks::ArtworksSnapshot>
    {
    public:
        ModifyArtworksCommand(Artworks::ArtworksSnapshot &&snapshot,
                              const std::shared_ptr<ICommandTemplate<Artworks::ArtworksSnapshot>> &editTemplate):
            TemplatedCommand(std::move(snapshot), editTemplate)
        {
            m_Count = m_Argument.size();
        }

        ModifyArtworksCommand(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork,
                              const std::shared_ptr<ICommandTemplate<Artworks::ArtworksSnapshot>> &editTemplate):
            TemplatedCommand(Artworks::ArtworksSnapshot({artwork}), editTemplate),
            m_Count(1)
        { }

    public:
        virtual bool canUndo() override { return true; }
        virtual QString getDescription() const override {
            return m_Count > 1 ? QObject::tr("%1 items modified").arg(m_Count) :
                                 QObject::tr("1 item modified");
        }

    private:
        size_t m_Count;
    };
}

#endif // COMBINEDEDITCOMMAND_H
