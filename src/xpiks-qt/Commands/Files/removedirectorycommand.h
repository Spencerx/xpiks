/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef REMOVEDIRECTORYCOMMAND_H
#define REMOVEDIRECTORYCOMMAND_H

#include <memory>

#include <QObject>
#include <QString>

#include "Commands/Files/removefilescommandbase.h"
#include "Models/Artworks/artworkslistoperations.h"

#ifdef CORE_TESTS
#include "Filesystem/ifilescollection.h"
#endif

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Models {
    class ArtworksListModel;
    class ArtworksRepository;
    class SettingsModel;
}

namespace Commands {
    class ICommand;
    template <typename T> class ICommandTemplate;

    class RemoveDirectoryCommand:
            public QObject,
            public RemoveFilesCommandBase
    {
        Q_OBJECT
        using ArtworksCommandTemplate = std::shared_ptr<ICommandTemplate<Artworks::ArtworksSnapshot>>;
    public:
        RemoveDirectoryCommand(int originalIndex,
                               Models::ArtworksListModel &artworksList,
                               Models::ArtworksRepository &artworksRepository,
                               Models::SettingsModel &settingsModel,
                               std::shared_ptr<ICommand> const &saveSessionCommand = std::shared_ptr<ICommand>(),
                               ArtworksCommandTemplate const &addedArtworksTemplate = ArtworksCommandTemplate());

        // RemoveFilesCommandBase interface
    protected:
        virtual Models::ArtworksRemoveResult removeFiles() override;

    signals:
        void artworksAdded(int imagesCount, int vectorsCount);

#ifdef CORE_TESTS
    public:
        void setFakeFullDirectoryFiles(std::shared_ptr<Filesystem::IFilesCollection> const &files) { m_FakeFiles = files; }
#endif

    protected:
        virtual void restoreFiles() override;

    private:
        int m_DirectoryIndex;
        QString m_DirectoryPath;
        Models::SettingsModel &m_SettingsModel;
        std::shared_ptr<ICommandTemplate<Artworks::ArtworksSnapshot>> m_AddedArtworksTemplate;
        bool m_IsFullDirectory;
#ifdef CORE_TESTS
        std::shared_ptr<Filesystem::IFilesCollection> m_FakeFiles;
#endif
    };
}

#endif // REMOVEDIRECTORYCOMMAND_H
