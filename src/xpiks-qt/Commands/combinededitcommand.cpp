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
#include "../Artworks/artworkmetadata.h"
#include "../Common/flags.h"
#include "../Models/settingsmodel.h"
#include "../Common/defines.h"
#include "../Artworks/artworkssnapshot.h"
#include "../Artworks/iartworksservice.h"

namespace Commands {
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

    QString getModificationTypeDescription(CombinedEditCommand::ModificationType type) {
        switch (type) {
        case CombinedEditCommand::PasteModificationType:
            return QObject::tr("Paste");
        case CombinedEditCommand::CombinedEditModificationType:
            return QObject::tr("Multiple edit");
        default:
            return QLatin1String("");
        }
    }

    CombinedEditCommand::~CombinedEditCommand() {
        LOG_DEBUG << "#";
    }

    std::shared_ptr<CommandResult> CombinedEditCommand::execute(int commandID) {
        m_CommandID = commandID;
        LOG_INFO << "flags =" << combinedFlagsToString(m_EditFlags) << ", artworks count =" << m_RawSnapshot.size();
        const size_t size = m_RawSnapshot.size();
        m_ArtworksBackups.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            auto &locker = m_RawSnapshot.at(i);
            Artworks::ArtworkMetadata *artwork = locker->getArtworkMetadata();

            m_ArtworksBackups.emplace_back(UndoRedo::ArtworkMetadataBackup(artwork));

            setKeywords(artwork);
            setDescription(artwork);
            setTitle(artwork);
        }

        if (m_BackupService) {
            m_BackupService->submitArtworks();
        }

        return IUndoCommand::execute(commandID);
    }

    QString CombinedEditCommand::getDescription() const {
        size_t count = m_ArtworksBackups.size();
        QString typeStr = getModificationTypeDescription(m_ModificationType);
        return count > 1 ? QObject::tr("(%1)  %2 items modified").arg(typeStr).arg(count) :
                             QObject::tr("(%1)  1 item modified").arg(typeStr);
    }

    void CombinedEditCommand::setKeywords(Artworks::ArtworkMetadata *artwork) const {
        if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::EditKeywords)) {
            if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::AppendKeywords)) {
                artwork->appendKeywords(m_Keywords);
            }
            else {
                if (Common::HasFlag(m_EditFlags,Common:: CombinedEditFlags::Clear)) {
                    artwork->clearKeywords();
                } else {
                    artwork->setKeywords(m_Keywords);
                }
            }
        }
    }

    void CombinedEditCommand::setDescription(Artworks::ArtworkMetadata *artwork) const {
        if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::EditDescription)) {
            if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::Clear)) {
                artwork->setDescription("");
            } else {
                artwork->setDescription(m_ArtworkDescription);
            }
        }
    }

    void CombinedEditCommand::setTitle(Artworks::ArtworkMetadata *artwork) const {
        if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::EditTitle)) {
            if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::Clear)) {
                artwork->setTitle("");
            } else {
                artwork->setTitle(m_ArtworkTitle);
            }
        }
    }

    void CombinedEditCommandResult::afterExecCallback(const ICommandManager *commandManagerInterface) {
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
}
