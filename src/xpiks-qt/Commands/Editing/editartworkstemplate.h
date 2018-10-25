/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef EDITARTWORKSTEMPLATE_H
#define EDITARTWORKSTEMPLATE_H

#include <memory>
#include <vector>

#include <QString>
#include <QStringList>

#include "Commands/Base/icommandtemplate.h"
#include "Common/flags.h"
#include "UndoRedo/artworkmetadatabackup.h"

namespace Artworks {
    class ArtworkMetadata;
    class ArtworksSnapshot;
}

namespace Commands {
    class EditArtworksTemplate: public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        EditArtworksTemplate(const QString &title,
                             const QString &description,
                             const QStringList &keywords,
                             Common::ArtworkEditFlags editFlags);
        EditArtworksTemplate(Common::ArtworkEditFlags editFlags);

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(const Artworks::ArtworksSnapshot &snapshot) override;
        virtual void undo(const Artworks::ArtworksSnapshot &snapshot) override;

    private:
        void editKeywords(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) const;
        void editDescription(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) const;
        void editTitle(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) const;

    private:
        QString m_Title;
        QString m_Description;
        QStringList m_Keywords;
        Common::ArtworkEditFlags m_EditFlags;
        std::vector<UndoRedo::ArtworkMetadataBackup> m_ArtworksBackups;
    };
}

#endif // EDITARTWORKSTEMPLATE_H
