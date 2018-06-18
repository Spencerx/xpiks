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
#include "../Common/flags.h"
#include "../Filesystem/ifilescollection.h"

namespace Models {
    class ArtworksListModel;
    class RecentFilesModel;
    class ArtworksRepository;
    class SettingsModel;
    class SwitcherModel;
}

namespace MetadataIO {
    class MetadataIOService;
    class MetadataIOCoordinator;
}

namespace QMLExtensions {
    class ImageCachingService;
    class VideoCachingService;
}

namespace UndoRedo {
    class UndoRedoManager;
}

namespace Commands {
    class AddFilesCommand:
            public QObject,
            public IUndoCommand
    {
        Q_OBJECT
    public:
        AddFilesCommand(std::shared_ptr<Filesystem::IFilesCollection> &files,
                        Common::AddFilesFlags flags,
                        std::shared_ptr<Commands::ICommand> &saveSessionCommand,
                        std::shared_ptr<Commands::ICommand> &clearLegacyBackupsCommand,
                        Models::ArtworksListModel &artworksListModel,
                        Models::ArtworksRepository &artworksRepository,
                        Models::SettingsModel &settingsModel,
                        Models::SwitcherModel &switcherModel,
                        MetadataIO::MetadataIOService &metadataIOService,
                        MetadataIO::MetadataIOCoordinator &metadataIOCoordinator,
                        QMLExtensions::ImageCachingService &imageCachingService,
                        QMLExtensions::VideoCachingService &videoCachingService,
                        Models::RecentFilesModel &recentFileModel):
            QObject(),
            m_Files(std::move(files)),
            m_Flags(flags),
            m_SaveSessionCommand(std::move(saveSessionCommand)),
            m_ClearLegacyBackupsCommand(std::move(clearLegacyBackupsCommand)),
            m_ArtworksListModel(artworksListModel),
            m_ArtworksRepository(artworksRepository),
            m_SettingsModel(settingsModel),
            m_SwitcherModel(switcherModel),
            m_MetadataIOService(metadataIOService),
            m_MetadataIOCoordinator(metadataIOCoordinator),
            m_ImageCachingService(imageCachingService),
            m_VideoCachingService(videoCachingService),
            m_RecentFileModel(recentFileModel)
        { }

        // ICommand interface
    public:
        virtual std::shared_ptr<CommandResult> execute(int commandID) override;

    private:
        int addFiles();
        void saveSession();

        // IHistoryItem interface
    public:
        virtual void undo() override;
        virtual QString getDescription() const override {
            return m_AddedCount != 1 ? QObject::tr("%1 items added").arg(m_AddedCount) :
                                       QObject::tr("1 item added");
        }
        virtual int getCommandID() const override { return m_CommandID; }

    signals:
        void artworksAdded(int importID, int imagesCount, int vectorsCount);

    private:
        std::shared_ptr<Filesystem::IFilesCollection> m_Files;
        Common::AddFilesFlags m_Flags = Common::AddFilesFlags::None;
        int m_CommandID;
        std::shared_ptr<Commands::ICommand> m_SaveSessionCommand;
        std::shared_ptr<Commands::ICommand> m_ClearLegacyBackupsCommand;
        Models::ArtworksListModel &m_ArtworksListModel;
        Models::ArtworksRepository &m_ArtworksRepository;
        Models::SettingsModel &m_SettingsModel;
        Models::SwitcherModel &m_SwitcherModel;
        MetadataIO::MetadataIOService &m_MetadataIOService;
        MetadataIO::MetadataIOCoordinator &m_MetadataIOCoordinator;
        QMLExtensions::ImageCachingService &m_ImageCachingService;
        QMLExtensions::VideoCachingService &m_VideoCachingService;
        Models::RecentFilesModel &m_RecentFileModel;
        // undo
        int m_OriginalCount = 0;
        int m_AddedCount = 0;
    };
}

#endif // ADDFILESCOMMAND_H
