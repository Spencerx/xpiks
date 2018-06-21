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

#include <QStringList>
#include <QString>
#include <vector>
#include <memory>
#include "icommand.h"
#include "../Common/flags.h"
#include "../Artworks/artworkssnapshot.h"
#include "../UndoRedo/artworkmetadatabackup.h"

namespace Artworks {
    class ArtworkMetadata;
    class IArtworksService;
}

namespace Commands {
    class CombinedEditCommand: public IUndoCommand
    {
        enum ModificationType {
            PasteModificationType,
            CombinedEditModificationType
        };

    public:
        CombinedEditCommand(Common::CombinedEditFlags editFlags,
                            Artworks::ArtworksSnapshot &rawSnapshot,
                            const QString &description,
                            const QString &title,
                            const QStringList &keywords,
                            const std::shared_ptr<Artworks::IArtworksService> &backupService,
                            const std::shared_ptr<Artworks::IArtworksService> &inspectionService):
            m_RawSnapshot(std::move(rawSnapshot)),
            m_ArtworkDescription(description),
            m_ArtworkTitle(title),
            m_Keywords(keywords),
            m_EditFlags(editFlags),
            m_BackupService(backupService),
            m_InspectionService(inspectionService)
        { }

        virtual ~CombinedEditCommand();

    public:
        virtual std::shared_ptr<CommandResult> execute(int commandID) override;
        virtual void undo() override;
        virtual QString getDescription() const override;
        virtual int getCommandID() const { return m_CommandID; }

    private:
        void setKeywords(Artworks::ArtworkMetadata *artwork) const;
        void setDescription(Artworks::ArtworkMetadata *artwork) const;
        void setTitle(Artworks::ArtworkMetadata *artwork) const;

    private:
        Artworks::ArtworksSnapshot m_Snapshot;
        std::vector<UndoRedo::ArtworkMetadataBackup> m_ArtworksBackups;
        QString m_ArtworkDescription;
        QString m_ArtworkTitle;
        QStringList m_Keywords;
        Common::CombinedEditFlags m_EditFlags;
        std::shared_ptr<Artworks::IArtworksService> m_BackupService;
        std::shared_ptr<Artworks::IArtworksService> m_InspectionService;
        int m_CommandID;
    };
}

#endif // COMBINEDEDITCOMMAND_H
