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
#include "icommand.h"
#include "iartworkscommandtemplate.h"
#include "../Common/flags.h"
#include "../Filesystem/ifilescollection.h"

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
        AddFilesCommand(std::shared_ptr<Filesystem::IFilesCollection> &files,
                        Common::AddFilesFlags flags,
                        Models::ArtworksListModel &artworksListModel,
                        std::shared_ptr<IArtworksCommandTemplate> &addedArtworksTemplate);

        // ICommand interface
    public:
        virtual void execute() override;

        // IHistoryItem interface
    public:
        virtual void canUndo() override { return true; }
        virtual void undo() override;
        virtual QString getDescription() const override {
            return m_AddedCount != 1 ? QObject::tr("%1 items added").arg(m_AddedCount) :
                                       QObject::tr("1 item added");
        }

    signals:
        void artworksAdded(int imagesCount, int vectorsCount);

    private:
        std::shared_ptr<Filesystem::IFilesCollection> m_Files;
        Common::AddFilesFlags m_Flags = Common::AddFilesFlags::None;
        Models::ArtworksListModel &m_ArtworksListModel;
        std::shared_ptr<IArtworksCommandTemplate> m_AddedArtworksTemplate;
        // undo
        int m_OriginalCount = 0;
        int m_AddedCount = 0;
    };
}

#endif // ADDFILESCOMMAND_H
