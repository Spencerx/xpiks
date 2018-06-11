/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ADDARTWORKSCOMMAND_H
#define ADDARTWORKSCOMMAND_H

#include <QStringList>
#include <QHash>
#include <QSet>
#include "commandbase.h"
#include "../Common/flags.h"
#include "../MetadataIO/artworkssnapshot.h"

namespace MetadataIO {
    class ArtworksSnapshot;
}

namespace Models {
    class ArtworksRepository;
    class ArtItemsModel;
}

namespace Commands {
    class CommandManager;

    class AddArtworksCommand : public CommandBase
    {
    public:
        enum AddArtworksFlags {
            FlagAutoFindVectors = 1 << 0,
            FlagIsFullDirectory = 1 << 1,
            FlagIsSessionRestore = 1 << 2,
            FlagAutoImport = 1 << 3
        };

    private:
        inline bool getAutoFindVectorsFlag() const { return Common::HasFlag(m_Flags, FlagAutoFindVectors); }
        inline bool getIsFullDirectoryFlag() const { return Common::HasFlag(m_Flags, FlagIsFullDirectory); }
        inline bool getIsSessionRestoreFlag() const { return Common::HasFlag(m_Flags, FlagIsSessionRestore); }
        inline bool getAutoImportFlag() const { return Common::HasFlag(m_Flags, FlagAutoImport); }

    public:
        AddArtworksCommand(const QStringList &pathes, const QStringList &vectorPathes, Common::flag_t flags) :
            CommandBase(CommandType::AddArtworks),
            m_FilePathes(pathes),
            m_VectorsPathes(vectorPathes),
            m_Flags(flags)
        { }

        virtual ~AddArtworksCommand();

    public:
        virtual std::shared_ptr<ICommandResult> execute(const ICommandManager *commandManagerInterface) override;

    private:
        void addFilesToAdd(int newFilesCount,
                           Models::ArtworksRepository *artworksRepository,
                           Models::ArtItemsModel *artItemsModel);
        void cleanupOldBackups(QSet<qint64> &directoryIDs, Models::ArtworksRepository *artworksRepository);
        int afterAddedHandler(CommandManager *commandManager,
                              const MetadataIO::ArtworksSnapshot &artworksToImport,
                              QStringList filesToWatch,
                              int initialCount, int newFilesCount) const;
        void decomposeVectors(QHash<QString, QHash<QString, QString> > &vectors) const;

    public:
        MetadataIO::ArtworksSnapshot m_ArtworksToImport;
        QSet<qint64> m_DirectoryIDs;
        QStringList m_FilesToWatch;
        QStringList m_FilePathes;
        QStringList m_VectorsPathes;
        Common::flag_t m_Flags;
    };

    class AddArtworksCommandResult : public CommandResult {
    public:
        AddArtworksCommandResult(
                Models::ArtworksRepository &artworksRepository,
                QSet<qint64> directoryIDs,
                int addedFilesCount,
                int attachedVectorsCount,
                int importID,
                bool autoImport):
            m_ArtworksRepository(artworksRepository),
            m_DirectoryIDs(directoryIDs),
            m_NewFilesAdded(addedFilesCount),
            m_AttachedVectorsCount(attachedVectorsCount),
            m_ImportID(importID),
            m_AutoImport(autoImport)
        { }

    public:
        virtual void afterExecCallback(const ICommandManager *commandManagerInterface) override;

    public:
        Models::ArtworksRepository &m_ArtworksRepository;
        QSet<qint64> m_DirectoryIDs;
        int m_NewFilesAdded;
        int m_AttachedVectorsCount;
        int m_ImportID;
        bool m_AutoImport;
    };
}

#endif // ADDARTWORKSCOMMAND_H
