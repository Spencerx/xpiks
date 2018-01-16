/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "combinededitcommand.h"
#include <QVector>
#include <QString>
#include <QStringList>
#include "../Commands/commandmanager.h"
#include "../UndoRedo/artworkmetadatabackup.h"
#include "../UndoRedo/modifyartworkshistoryitem.h"
#include "../Models/artworkmetadata.h"
#include "../Common/flags.h"
#include "../Models/settingsmodel.h"
#include "../Common/defines.h"
#include "../MetadataIO/artworkssnapshot.h"

QString combinedFlagsToString(Common::CombinedEditFlags flags) {
    using namespace Common;

    if (flags == CombinedEditFlags::EditEverything) {
        return QLatin1String("EditEverything");
    }

    QStringList flagsStr;
    if (Common::HasFlag(flags, CombinedEditFlags::EditDescription)) {
        flagsStr.append("EditDescription");
    }

    if (Common::HasFlag(flags, CombinedEditFlags::EditTitle)) {
        flagsStr.append("EditTitle");
    }

    if (Common::HasFlag(flags, CombinedEditFlags::EditKeywords)) {
        flagsStr.append("EditKeywords");
    }

    if (Common::HasFlag(flags, CombinedEditFlags::Clear)) {
        flagsStr.append("Clear");
    }

    return flagsStr.join('|');
}

Commands::CombinedEditCommand::~CombinedEditCommand() {
    LOG_DEBUG << "#";
}

std::shared_ptr<Commands::ICommandResult> Commands::CombinedEditCommand::execute(const ICommandManager *commandManagerInterface) const {
    LOG_INFO << "flags =" << combinedFlagsToString(m_EditFlags) << ", artworks count =" << m_RawSnapshot.size();
    QVector<int> indicesToUpdate;
    std::vector<UndoRedo::ArtworkMetadataBackup> artworksBackups;
    MetadataIO::WeakArtworksSnapshot itemsToSave, affectedItems;

    CommandManager *commandManager = (CommandManager*)commandManagerInterface;
    auto *xpiks = commandManager->getDelegator();

    size_t size = m_RawSnapshot.size();
    indicesToUpdate.reserve((int)size);
    artworksBackups.reserve(size);
    itemsToSave.reserve(size);
    affectedItems.reserve((int)size);

    for (size_t i = 0; i < size; ++i) {
        auto &locker = m_RawSnapshot.at(i);
        Models::ArtworkMetadata *artwork = locker->getArtworkMetadata();

        artworksBackups.emplace_back(artwork);
        indicesToUpdate.append((int)artwork->getLastKnownIndex());

        setKeywords(artwork);
        setDescription(artwork);
        setTitle(artwork);

        itemsToSave.push_back(artwork);
        affectedItems.push_back(artwork);
    }

    std::unique_ptr<UndoRedo::IHistoryItem> modifyArtworksItem(
                new UndoRedo::ModifyArtworksHistoryItem(
                    getCommandID(),
                    artworksBackups, indicesToUpdate,
                    UndoRedo::CombinedEditModificationType));
    xpiks->recordHistoryItem(modifyArtworksItem);

    std::shared_ptr<ICommandResult> result(new CombinedEditCommandResult(affectedItems, itemsToSave, indicesToUpdate));
    return result;
}

void Commands::CombinedEditCommand::setKeywords(Models::ArtworkMetadata *metadata) const {
    if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::EditKeywords)) {
        if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::AppendKeywords)) {
            metadata->appendKeywords(m_Keywords);
        }
        else {
            if (Common::HasFlag(m_EditFlags,Common:: CombinedEditFlags::Clear)) {
                metadata->clearKeywords();
            } else {
                metadata->setKeywords(m_Keywords);
            }
        }
    }
}

void Commands::CombinedEditCommand::setDescription(Models::ArtworkMetadata *metadata) const {
    if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::EditDescription)) {
        if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::Clear)) {
            metadata->setDescription("");
        } else {
            metadata->setDescription(m_ArtworkDescription);
        }
    }
}

void Commands::CombinedEditCommand::setTitle(Models::ArtworkMetadata *metadata) const {
    if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::EditTitle)) {
        if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::Clear)) {
            metadata->setTitle("");
        } else {
            metadata->setTitle(m_ArtworkTitle);
        }
    }
}

void Commands::CombinedEditCommandResult::afterExecCallback(const Commands::ICommandManager *commandManagerInterface) const {
    CommandManager *commandManager = (CommandManager*)commandManagerInterface;
    auto *xpiks = commandManager->getDelegator();

    if (!m_IndicesToUpdate.isEmpty()) {
        xpiks->updateArtworksAtIndices(m_IndicesToUpdate);
    }

    if (!m_ItemsToSave.empty()) {
        xpiks->saveArtworksBackups(m_ItemsToSave);
    }

    if (!m_AffectedItems.empty()) {
        xpiks->submitForSpellCheck(m_AffectedItems);
        xpiks->submitForWarningsCheck(m_AffectedItems);
    }
}
