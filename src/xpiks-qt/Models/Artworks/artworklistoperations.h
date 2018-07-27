#ifndef ARTWORKLISTOPERATIONS_H
#define ARTWORKLISTOPERATIONS_H

#include <QSet>
#include <Artworks/artworkssnapshot.h>

namespace Models {
    struct ArtworksAddResult {
        ArtworksAddResult(Artworks::ArtworksSnapshot &snapshot, int attachedVectorsCount):
            m_Snapshot(std::move(snapshot)),
            m_AttachedVectorsCount(attachedVectorsCount)
        { }
        Artworks::ArtworksSnapshot m_Snapshot;
        int m_AttachedVectorsCount;
    };

    struct ArtworksRemoveResult {
        QSet<qint64> m_SelectedDirectoryIds;
        size_t m_RemovedCount;
        bool m_UnselectAll;
    };
}

#endif // ARTWORKLISTOPERATIONS_H
