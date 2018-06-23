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

#include <QObject>
#include "artworkscommand.h"

namespace Commands {
    class ModifyArtworksCommand: public ArtworksCommand
    {
    public:
        ModifyArtworksCommand(const std::shared_ptr<Artworks::ArtworksSnapshot> &snapshot,
                            std::shared_ptr<IArtworksCommandTemplate> &editTemplate):
            ArtworksCommand(snapshot, editTemplate),
            m_Count(snapshot.size())
        { }

    public:
        virtual bool canUndo() override { return true; }

        virtual QString getDescription() const override {
            return m_Count > 1 ? QObject::tr("%1 items modified").arg(m_Count) :
                                 QObject::tr("1 item modified");
        }

    private:
        int m_Count;
    };
}

#endif // COMBINEDEDITCOMMAND_H
