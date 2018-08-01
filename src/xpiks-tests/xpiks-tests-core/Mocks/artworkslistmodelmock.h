#ifndef ARTITEMSMODELMOCK_H
#define ARTITEMSMODELMOCK_H

#include <QVector>
#include <QPair>
#include <functional>
#include <Models/Artworks/artworkslistmodel.h>
#include <Artworks/artworkmetadata.h>
#include "artworkmetadatamock.h"
#include "filescollectionmock.h"

namespace Mocks {
    class ArtworksListModelMock : public Models::ArtworksListModel
    {
    public:
        ArtworksListModelMock(): m_BlockUpdates(true) {}

    public:
        virtual Artworks::ArtworkMetadata *createArtwork(const QString &filepath, qint64 directoryID) override {
            ArtworkMetadataMock *artwork = new ArtworkMetadataMock(filepath, directoryID);
            artwork->initialize("Test title", "Test description", QStringList() << "keyword1" << "keyword2" << "keyword3");
            this->connectArtworkSignals(artwork);
            return artwork;
        }
        void setUpdatesBlocked(bool value) { m_BlockUpdates = value; }

        ArtworkMetadataMock *getMockArtwork(int index) const { return dynamic_cast<ArtworkMetadataMock*>(getArtwork(index)); }

        void removeAll() { deleteItems(Helpers::IndicesRanges(rowCount())); }

        void mockDeletion(int count) {
            for (int i = 0; i < count; ++i) {
                getArtwork(i)->setUnavailable();
            }
        }

        virtual void updateItems(const Helpers::IndicesRanges &ranges, const QVector<int> &roles = QVector<int>()) override {
            if (!m_BlockUpdates) {
                Models::ArtworksListModel::updateItems(ranges, roles);
            }
        }

        void foreachArtwork(std::function<void (int index, ArtworkMetadataMock *metadata)> action) {
            int size = getArtworksCount();
            for (int i = 0; i < size; ++i) {
                auto *item = dynamic_cast<ArtworkMetadataMock*>(getArtwork(i));
                action(i, item);
            }
        }

        void generateAndAddArtworks(size_t count, bool withVector=true) {
            generateAndAddArtworksEx(count, 2, withVector);
        }

        void generateAndAddArtworksEx(size_t count, int dirsCount, bool withVector) {
            Q_ASSERT(count >= 0);
            Q_ASSERT(dirsCount > 0);
            addFiles(std::make_shared<Mocks::FilesCollectionMock>(count, withVector ? count : 0, dirsCount),
                     Common::AddFilesFlags::None);
        }

    private:
        bool m_BlockUpdates;
    };
}

#endif // ARTITEMSMODELMOCK_H
