/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "editartworkstemplate.h"

#include <cstddef>

#include <QLatin1String>
#include <QtDebug>
#include <QtGlobal>

#include "Artworks/artworkmetadata.h"
#include "Artworks/artworkssnapshot.h"
#include "Common/flags.h"
#include "Common/logging.h"
#include "UndoRedo/artworkmetadatabackup.h"

namespace Commands {
    QString combinedFlagsToString(Common::ArtworkEditFlags flags) {
        using namespace Common;

        if (flags == ArtworkEditFlags::EditEverything) {
            return QLatin1String("EditEverything");
        }

        QStringList flagsStr;
        if (Common::HasFlag(flags, ArtworkEditFlags::EditDescription)) {
            flagsStr.append("EditDescription");
        }

        if (Common::HasFlag(flags, ArtworkEditFlags::EditTitle)) {
            flagsStr.append("EditTitle");
        }

        if (Common::HasFlag(flags, ArtworkEditFlags::EditKeywords)) {
            flagsStr.append("EditKeywords");
        }

        if (Common::HasFlag(flags, ArtworkEditFlags::Clear)) {
            flagsStr.append("Clear");
        }

        return flagsStr.join('|');
    }

    EditArtworksTemplate::EditArtworksTemplate(const QString &title,
                                               const QString &description,
                                               const QStringList &keywords,
                                               Common::ArtworkEditFlags editFlags):
        m_Title(title),
        m_Description(description),
        m_Keywords(keywords),
        m_EditFlags(editFlags)
    {
    }

    EditArtworksTemplate::EditArtworksTemplate(Common::ArtworkEditFlags editFlags):
        m_EditFlags(editFlags)
    {
    }

    void EditArtworksTemplate::execute(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_INFO << "flags =" << combinedFlagsToString(m_EditFlags) << ", artworks count =" << snapshot.size();
        const size_t size = snapshot.size();
        m_ArtworksBackups.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            auto &artwork = snapshot.get(i);

            m_ArtworksBackups.emplace_back(UndoRedo::ArtworkMetadataBackup(artwork));

            editKeywords(artwork);
            editDescription(artwork);
            editTitle(artwork);
        }
    }

    void EditArtworksTemplate::undo(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "#";
        Q_ASSERT(snapshot.size() == m_ArtworksBackups.size());
        const size_t size = snapshot.size();
        for (size_t i = 0; i < size; i++) {
            auto &artwork = snapshot.get(i);
            auto &backup = m_ArtworksBackups.at(i);
            backup.restore(artwork);
        }
    }

    void EditArtworksTemplate::editKeywords(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) const {
        if (Common::HasFlag(m_EditFlags, Common::ArtworkEditFlags::EditKeywords)) {
            if (Common::HasFlag(m_EditFlags, Common::ArtworkEditFlags::AppendKeywords)) {
                artwork->appendKeywords(m_Keywords);
            } else {
                if (Common::HasFlag(m_EditFlags,Common:: ArtworkEditFlags::Clear)) {
                    artwork->clearKeywords();
                } else {
                    artwork->setKeywords(m_Keywords);
                }
            }
        }
    }

    void EditArtworksTemplate::editDescription(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) const {
        if (Common::HasFlag(m_EditFlags, Common::ArtworkEditFlags::EditDescription)) {
            if (Common::HasFlag(m_EditFlags, Common::ArtworkEditFlags::Clear)) {
                artwork->setDescription("");
            } else {
                artwork->setDescription(m_Description);
            }
        }
    }

    void EditArtworksTemplate::editTitle(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) const {
        if (Common::HasFlag(m_EditFlags, Common::ArtworkEditFlags::EditTitle)) {
            if (Common::HasFlag(m_EditFlags, Common::ArtworkEditFlags::Clear)) {
                artwork->setTitle("");
            } else {
                artwork->setTitle(m_Title);
            }
        }
    }
}
