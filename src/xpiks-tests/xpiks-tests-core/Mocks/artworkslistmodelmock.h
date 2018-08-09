#ifndef ArtworksListModelMock_H
#define ArtworksListModelMock_H

#include <QVector>
#include <tuple>
#include <functional>
#include <Models/Artworks/artworkslistmodel.h>
#include <Artworks/artworkmetadata.h>
#include "artworkmetadatamock.h"
#include "filescollectionmock.h"

namespace Mocks {
    class ArtworksListModelMock : public Models::ArtworksListModel
    {
    public:
        ArtworksListModelMock(Models::ArtworksRepository &repository):
            Models::ArtworksListModel(repository),
            m_BlockUpdates(true)
        { }

    public:
        virtual Artworks::ArtworkMetadata *createArtwork(const Filesystem::ArtworkFile &file, qint64 directoryID) override {
            ArtworkMetadataMock *artwork = new ArtworkMetadataMock(file.m_Path, directoryID);
            artwork->initialize("Test title", "Test description", QStringList() << "keyword1" << "keyword2" << "keyword3");
            return artwork;
        }
        void setUpdatesBlocked(bool value) { m_BlockUpdates = value; }

        ArtworkMetadataMock *getMockArtwork(int index) const { return dynamic_cast<ArtworkMetadataMock*>(getArtwork(index)); }
        size_t getFinalizationListSize() const { return getFinalizationList().size(); }
        void removeAll() { deleteItems(Helpers::IndicesRanges(getArtworksSize())); }

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
            int size = getArtworksSize();
            for (int i = 0; i < size; ++i) {
                auto *item = dynamic_cast<ArtworkMetadataMock*>(getArtwork(i));
                action(i, item);
            }
        }

        int generateAndAddArtworks(int count, bool withVector=true) {
            return generateAndAddArtworksEx(count, 2, withVector);
        }

        int generateAndAddArtworksEx(int count,
                                      int dirsCount,
                                      bool withVector,
                                      Common::AddFilesFlags flags = Common::AddFilesFlags::None) {
            Q_ASSERT(count >= 0);
            Q_ASSERT(dirsCount > 0);
            auto result = addFiles(std::make_shared<Mocks::FilesCollectionMock>(count, withVector ? count : 0, dirsCount), flags);
            return (int)result.m_Snapshot.size();
        }

        std::tuple<int, std::shared_ptr<Filesystem::IFilesCollection>> generateAndAddDirectories(int dirsCount, int fileInDir = 5, bool withVector=true) {
            auto files = std::make_shared<Mocks::FilesCollectionMock>(fileInDir*dirsCount,
                                                                      withVector ? fileInDir*dirsCount : 0,
                                                                      dirsCount);
            files->setFromFullDirectory();
            auto result = addFiles(files, Common::AddFilesFlags::FlagIsFullDirectory);
            return {(int)result.m_Snapshot.size(), files};
        }

        size_t getAvailableArtworksSize() const {
            return filterAvailableArtworks<size_t>([](Artworks::ArtworkMetadata*,size_t index) { return index; }).size();
        }

    private:
        bool m_BlockUpdates;
    };
}

#endif // ArtworksListModelMock_H
