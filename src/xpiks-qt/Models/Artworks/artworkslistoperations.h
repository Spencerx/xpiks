#ifndef ARTWORKSLISTOPERATIONS_H
#define ARTWORKSLISTOPERATIONS_H

#include <cstddef>
#include <utility>

#include <QSet>
#include <QtGlobal>

#include "Artworks/artworkssnapshot.h"
#include "Helpers/indicesranges.h"

namespace Models {
    struct ArtworksAddResult {
        ArtworksAddResult():
            m_AttachedVectorsCount(0)
        { }

        ArtworksAddResult(Artworks::ArtworksSnapshot &snapshot, int attachedVectorsCount):
            m_Snapshot(std::move(snapshot)),
            m_AttachedVectorsCount(attachedVectorsCount)
        { }
        Artworks::ArtworksSnapshot m_Snapshot;
        int m_AttachedVectorsCount = 0;
    };

    struct ArtworksRemoveResult {
        Helpers::IndicesRanges m_RemovedRanges;
        QSet<qint64> m_SelectedDirectoryIds;
        QSet<qint64> m_FullDirectoryIds;
        size_t m_RemovedCount = 0;
        bool m_UnselectAll = false;
    };
}

#endif // ARTWORKSLISTOPERATIONS_H
