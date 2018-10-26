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

#include <memory>

namespace Artworks {
    class ArtworkMetadata;
}

namespace Suggestion {
    class LocalLibraryQuery;
}

namespace MetadataIO {
    class MetadataIOTaskBase
    {
    public:
        virtual ~MetadataIOTaskBase() {}
    };

    class MetadataSearchTask: public MetadataIOTaskBase {
    public:
        MetadataSearchTask(Suggestion::LocalLibraryQuery *query):
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
        MetadataReadWriteTask(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork, ReadWriteAction readWriteAction):
            m_Artwork(artwork),
            m_ReadWriteAction(readWriteAction)
        {}

    public:
        ReadWriteAction getReadWriteAction() const { return m_ReadWriteAction; }
        std::shared_ptr<Artworks::ArtworkMetadata> const &getArtworkMetadata() const { return m_Artwork; }

    private:
        std::shared_ptr<Artworks::ArtworkMetadata> m_Artwork;
        ReadWriteAction m_ReadWriteAction;
    };
}

#endif // METADATAIOTASK_H
