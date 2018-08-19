#include "undoaddwithvectorstest.h"
#include <QUrl>
#include <QList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString UndoAddWithVectorsTest::testName() {
    return QLatin1String("UndoAddWithVectorsTest");
}

void UndoAddWithVectorsTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(true);

    // copy files
    setupFilePathForTest("images-for-tests/mixed/026.eps");
    setupFilePathForTest("images-for-tests/mixed/027.eps");
}

int UndoAddWithVectorsTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/mixed/026.jpg");
    files << setupFilePathForTest("images-for-tests/mixed/0267.jpg");
    files << setupFilePathForTest("images-for-tests/mixed/027.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto firstImage = std::dynamic_pointer_cast<Artworks::ImageArtwork>(m_TestsApp.getArtwork(0));
    Q_ASSERT(firstImage != NULL);

    auto secondImage = std::dynamic_pointer_cast<Artworks::ImageArtwork>(m_TestsApp.getArtwork(1));
    Q_ASSERT(secondImage != NULL);

    auto thirdImage = std::dynamic_pointer_cast<Artworks::ImageArtwork>(m_TestsApp.getArtwork(2));
    Q_ASSERT(thirdImage != NULL);

    VERIFY(firstImage->hasVectorAttached(), "Vector wasn't attached to an image with vector");
    QString firstVector = firstImage->getAttachedVectorPath();

    VERIFY(!secondImage->hasVectorAttached(), "Vector was attached to an image without vector");

    VERIFY(thirdImage->hasVectorAttached(), "Vector wasn't attached to an image with vector");
    QString thirdVector = thirdImage->getAttachedVectorPath();

    // remove directory
    m_TestsApp.deleteArtworksFromDirectory(0);
    VERIFY(m_TestsApp.getArtworksCount() == 0, "All items were not removed");

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForImport(waiter);
    VERIFY(m_TestsApp.undoLastAction(), "Failed to Undo last action");
    VERIFY(m_TestsApp.continueReading(waiter), "Failed to reimport files");
    VERIFY(m_TestsApp.getArtworksCount() == files.length(), "Items were not put back");

    firstImage = std::dynamic_pointer_cast<Artworks::ImageArtwork>(m_TestsApp.getArtwork(0));
    Q_ASSERT(firstImage != NULL);

    secondImage = std::dynamic_pointer_cast<Artworks::ImageArtwork>(m_TestsApp.getArtwork(1));
    Q_ASSERT(secondImage != NULL);

    thirdImage = std::dynamic_pointer_cast<Artworks::ImageArtwork>(m_TestsApp.getArtwork(2));
    Q_ASSERT(thirdImage != NULL);

    VERIFY(firstImage->hasVectorAttached(), "Vector wasn't attached to an image with vector");
    VERIFY(firstImage->getAttachedVectorPath() == firstVector, "Vector paths do not match for first item!")
    VERIFY(!secondImage->hasVectorAttached(), "Vector was attached to an image without vector");
    VERIFY(thirdImage->hasVectorAttached(), "Vector wasn't attached to an image with vector");
    VERIFY(thirdImage->getAttachedVectorPath() == thirdVector, "Vector paths do not match for third item");
    VERIFY(thirdVector != firstVector, "Vector got messed up");

    return 0;
}


