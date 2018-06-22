/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworkmetadatabackup.h"
#include "../Artworks/artworkmetadata.h"
#include "../Artworks/imageartwork.h"
#include "../Common/defines.h"

UndoRedo::ArtworkMetadataBackup::ArtworkMetadataBackup(Artworks::ArtworkMetadata *artwork) {
    m_ArtworkID = artwork->getItemID();
    m_Description = artwork->getDescription();
    m_Title = artwork->getTitle();
    m_KeywordsList = artwork->getKeywords();
    m_IsModified = artwork->isModified();

    Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork *>(artwork);
    if (image != NULL && image->hasVectorAttached()) {
        m_AttachedVector = image->getAttachedVectorPath();
    }
}

UndoRedo::ArtworkMetadataBackup::ArtworkMetadataBackup(const UndoRedo::ArtworkMetadataBackup &copy):
    m_ArtworkID(copy.m_ArtworkID),
    m_Description(copy.m_Description),
    m_Title(copy.m_Title),
    m_AttachedVector(copy.m_AttachedVector),
    m_KeywordsList(copy.m_KeywordsList),
    m_IsModified(copy.m_IsModified)
{
}

void UndoRedo::ArtworkMetadataBackup::restore(Artworks::ArtworkMetadata *artwork) const {
    Q_ASSERT(m_ArtworkID == artwork->getItemID());
    if (m_ArtworkID != artwork->getItemID()) {
        LOG_WARNING << "Cannot restore to different artwork";
        return;
    }
    artwork->setDescription(m_Description);
    artwork->setTitle(m_Title);
    artwork->setKeywords(m_KeywordsList);
    if (m_IsModified) { artwork->setModified(); }
    else { artwork->resetModified(); }

    if (!m_AttachedVector.isEmpty()) {
        Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork *>(artwork);
        if (image != NULL) {
            image->attachVector(m_AttachedVector);
        } else {
            LOG_WARNING << "Inconsistency for attached vector";
        }
    }
}
