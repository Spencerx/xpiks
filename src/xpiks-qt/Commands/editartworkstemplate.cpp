/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "editartworkstemplate.h"
#include "../Artworks/artworkmetadata.h"
#include "../Common/logging.h"
#include "../Artworks/artworkssnapshot.h"

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

    EditArtworksTemplate::EditArtworksTemplate(const QString &title,
                                               const QString &description,
                                               const QString &keywords,
                                               Common::CombinedEditFlags editFlags):
        m_Title(title),
        m_Description(description),
        m_Keywords(keywords),
        m_EditFlags(editFlags)
    {
    }

    void EditArtworksTemplate::execute(Artworks::ArtworksSnapshot &snapshot) {
        LOG_INFO << "flags =" << combinedFlagsToString(m_EditFlags) << ", artworks count =" << snapshot.size();
        const size_t size = snapshot.size();
        m_ArtworksBackups.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            Artworks::ArtworkMetadata *artwork = snapshot.get(i);

            m_ArtworksBackups.emplace_back(UndoRedo::ArtworkMetadataBackup(artwork));

            setKeywords(artwork);
            setDescription(artwork);
            setTitle(artwork);
        }
    }

    void EditArtworksTemplate::undo(Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "#";
        Q_ASSERT(snapshot.size() == m_ArtworksBackups.size());
        const size_t size = snapshot.size();
        for (size_t i = 0; i < size; i++) {
            Artworks::ArtworkMetadata *artwork = snapshot.get(i);
            auto &backup = m_ArtworksBackups.at(i);
            backup.restore(artwork);
        }
    }

    void EditArtworksTemplate::setKeywords(Artworks::ArtworkMetadata *artwork) const {
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

    void EditArtworksTemplate::setDescription(Artworks::ArtworkMetadata *artwork) const {
        if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::EditDescription)) {
            if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::Clear)) {
                artwork->setDescription("");
            } else {
                artwork->setDescription(m_Description);
            }
        }
    }

    void EditArtworksTemplate::setTitle(Artworks::ArtworkMetadata *artwork) const {
        if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::EditTitle)) {
            if (Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::Clear)) {
                artwork->setTitle("");
            } else {
                artwork->setTitle(m_Title);
            }
        }
    }
}
