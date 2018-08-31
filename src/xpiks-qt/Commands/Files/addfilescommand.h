/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ADDFILESCOMMAND_H
#define ADDFILESCOMMAND_H

#include <memory>
#include <QObject>
#include <Commands/Base/icommand.h>
#include <Commands/Base/icommandtemplate.h>
#include <Common/flags.h>
#include <Filesystem/ifilescollection.h>
#include <Artworks/artworkssnapshot.h>

namespace Models {
    class ArtworksListModel;
}

namespace Commands {
    class AddFilesCommand:
            public QObject,
            public ICommand
    {
        Q_OBJECT
    public:
        using ArtworksCommandTemplate = std::shared_ptr<ICommandTemplate<Artworks::ArtworksSnapshot>>;

        AddFilesCommand(std::shared_ptr<Filesystem::IFilesCollection> const &files,
                        Common::AddFilesFlags flags,
                        Models::ArtworksListModel &artworksListModel,
                        ArtworksCommandTemplate const &addedArtworksTemplate = ArtworksCommandTemplate());

        // ICommand interface
    public:
        virtual void execute() override;

        // IHistoryItem interface
    public:
        virtual bool canUndo() override { return true; }
        virtual void undo() override;
        virtual QString getDescription() const override {
            return m_AddedCount != 1 ? QObject::tr("%1 items added").arg(m_AddedCount) :
                                       QObject::tr("1 item added");
        }

    public:
        int getAddedCount() const { return m_AddedCount; }

    signals:
        void artworksAdded(int imagesCount, int vectorsCount);

    private:
        std::shared_ptr<Filesystem::IFilesCollection> m_Files;
        Common::AddFilesFlags m_Flags = Common::AddFilesFlags::None;
        Models::ArtworksListModel &m_ArtworksListModel;
        std::shared_ptr<ICommandTemplate<Artworks::ArtworksSnapshot>> m_AddedArtworksTemplate;
        // undo
        size_t m_OriginalCount = 0;
        size_t m_AddedCount = 0;
    };
}

#endif // ADDFILESCOMMAND_H
