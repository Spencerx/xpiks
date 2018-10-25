/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DELETEKEYWORDSCOMMAND_H
#define DELETEKEYWORDSCOMMAND_H

#include <vector>

#include <QSet>

#include "Commands/Base/icommandtemplate.h"
#include "UndoRedo/artworkmetadatabackup.h"

class QString;

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Commands {
    class DeleteKeywordsTemplate : public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        DeleteKeywordsTemplate(const QSet<QString> &keywordsSet,
                               bool caseSensitive);

    public:
        virtual void execute(const Artworks::ArtworksSnapshot &snapshot) override;
        virtual void undo(const Artworks::ArtworksSnapshot &snapshot) override;

    private:
        QSet<QString> m_KeywordsSet;
        std::vector<UndoRedo::ArtworkMetadataBackup> m_ArtworksBackups;
        bool m_CaseSensitive;
    };
}

#endif // DELETEKEYWORDSCOMMAND_H
