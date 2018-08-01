#ifndef FILESCOLLECTIONMOCK_H
#define FILESCOLLECTIONMOCK_H

#include <QStringList>
#include <Filesystem/ifilescollection.h>

#ifdef Q_OS_WIN
    #define DIRECTORY_PATH "C:/path/to/some/directory"
#else
    #define DIRECTORY_PATH "/path/to/some/directory"
#endif

#define ARTWORK_JPG_PATTERN "artwork%2.jpg"

#define ARTWORK_PATH DIRECTORY_PATH "_%1/" ARTWORK_JPG_PATTERN
#define VECTOR_PATH DIRECTORY_PATH "_%1/artwork%2.eps"

namespace Mocks {
    class FilesCollectionMock: public Filesystem::IFilesCollection
    {
    public:
        FilesCollectionMock(QStringList const &images);
        FilesCollectionMock(QStringList const &images, QStringList const &vectors);
        FilesCollectionMock(int imagesCount, int vectorsCount = 0, int dirsCount = 1);

        // IFilesCollection interface
    public:
        virtual std::vector<Filesystem::ArtworkFile> const &getFiles() override { return m_Files; }

    private:
        std::vector<Filesystem::ArtworkFile> m_Files;
    };
}

#endif // FILESCOLLECTIONMOCK_H
