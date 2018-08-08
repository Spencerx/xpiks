/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef REMOVEDIRECTORYCOMMAND_H
#define REMOVEDIRECTORYCOMMAND_H

#include <QObject>
#include "removefilescommandbase.h"
#include <Commands/Base/icommandtemplate.h>

namespace Models {
    class SettingsModel;
}

namespace Commands {
    class RemoveDirectoryCommand:
            public QObject,
            public RemoveFilesCommandBase
    {
        Q_OBJECT
        using ArtworksCommandTemplate = std::shared_ptr<ICommandTemplate<Artworks::ArtworksSnapshot>>;
    public:
        RemoveDirectoryCommand(int directoryID,
                               Models::ArtworksListModel &artworksList,
                               Models::ArtworksRepository &artworksRepository,
                               Models::SettingsModel &settingsModel,
                               ArtworksCommandTemplate const &addedArtworksTemplate = ArtworksCommandTemplate());

        // ICommand interface
    public:
        virtual void execute() override;

    signals:
        void artworksAdded(int imagesCount, int vectorsCount);

    protected:
        virtual void restoreFiles() override;

    private:
        int m_DirectoryID;
        QString m_DirectoryPath;
        Models::SettingsModel &m_SettingsModel;
        std::shared_ptr<ICommandTemplate<Artworks::ArtworksSnapshot>> m_AddedArtworksTemplate;
        bool m_IsFullDirectory;
    };
}

#endif // REMOVEDIRECTORYCOMMAND_H
