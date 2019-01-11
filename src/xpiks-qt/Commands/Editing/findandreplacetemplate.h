/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef FINDANDREPLACECOMMAND_H
#define FINDANDREPLACECOMMAND_H

#include <vector>

#include <QString>

#include "Commands/Base/icommandtemplate.h"
#include "Common/flags.h"
#include "UndoRedo/artworkmetadatabackup.h"

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Commands {
    class FindAndReplaceTemplate: public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        FindAndReplaceTemplate(const QString &replaceWhat, const QString &replaceTo, Common::SearchFlags flags):
            m_ReplaceWhat(replaceWhat),
            m_ReplaceTo(replaceTo),
            m_Flags(flags)
        { }

    public:
        virtual void execute(const Artworks::ArtworksSnapshot &snapshot) override;
        virtual void undo(const Artworks::ArtworksSnapshot &snapshot) override;

    private:
        std::vector<UndoRedo::ArtworkMetadataBackup> m_ArtworksBackups;
        QString m_ReplaceWhat;
        QString m_ReplaceTo;
        Common::SearchFlags m_Flags;
    };
}

#endif // FINDANDREPLACE_H
