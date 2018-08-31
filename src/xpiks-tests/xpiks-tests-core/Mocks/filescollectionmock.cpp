#include "filescollectionmock.h"

Mocks::FilesCollectionMock::FilesCollectionMock(QStringList const &images) {
    for (auto &img: images) {
        m_Files.emplace_back(img, Filesystem::ArtworkFileType::Image);
    }
}

Mocks::FilesCollectionMock::FilesCollectionMock(QStringList const &images, QStringList const &vectors) {
    for (auto &img: images) {
        m_Files.emplace_back(img, Filesystem::ArtworkFileType::Image);
    }

    for (auto &vec: vectors) {
        m_Files.emplace_back(vec, Filesystem::ArtworkFileType::Vector);
    }
}

Mocks::FilesCollectionMock::FilesCollectionMock(int imagesCount, int vectorsCount, int dirsCount) {
    for (int i = 0; i < imagesCount; i++) {
        QString filename = QString(ARTWORK_PATH).arg(i%dirsCount).arg(i);
        m_Files.emplace_back(filename, Filesystem::ArtworkFileType::Image);
    }

    for (int i = 0; i < vectorsCount; i++) {
        QString vectorname = QString(VECTOR_PATH).arg(i%dirsCount).arg(i);
        m_Files.emplace_back(vectorname, Filesystem::ArtworkFileType::Vector);
    }
}

void Mocks::FilesCollectionMock::setFromFullDirectory() {
    for (auto &file: m_Files) {
        file.m_IsPartOfFullDirectory = true;
    }
}
