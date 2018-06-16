/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef METADATAIOTASK_H
#define METADATAIOTASK_H

#include "../Models/artworkmetadata.h"
#include "../Suggestion/locallibraryquery.h"

namespace MetadataIO {
    class MetadataIOTaskBase: public Artworks::ArtworkMetadataLocker
    {
    public:
        MetadataIOTaskBase(Artworks::ArtworkMetadata *metadata):
            Artworks::ArtworkMetadataLocker(metadata)
        {
        }
    };

    class MetadataSearchTask: public MetadataIOTaskBase {
    public:
        MetadataSearchTask(Suggestion::LocalLibraryQuery *query):
            MetadataIOTaskBase(nullptr),
            m_Query(query)
        {
        }

    public:
        Suggestion::LocalLibraryQuery *getQuery() const { return m_Query; }

    private:
        Suggestion::LocalLibraryQuery *m_Query;
    };

    class MetadataReadWriteTask: public MetadataIOTaskBase {
    public:
        enum ReadWriteAction {
            Read,
            Write,
            Add
        };

    public:
        MetadataReadWriteTask(Artworks::ArtworkMetadata *metadata, ReadWriteAction readWriteAction):
            MetadataIOTaskBase(metadata),
            m_ReadWriteAction(readWriteAction)
        {}

    public:
        ReadWriteAction getReadWriteAction() const { return m_ReadWriteAction; }

    private:
        ReadWriteAction m_ReadWriteAction;
    };
}

#endif // METADATAIOTASK_H
