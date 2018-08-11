#include "clearmetadatatest.h"
#include <QUrl>
#include <QFileInfo>
#include <QStringList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString ClearMetadataTest::testName() {
    return QLatin1String("ClearMetadataTest");
}

void ClearMetadataTest::setup() {
    m_TestsApp.setAutoFindVector(false);
}

int ClearMetadataTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull-for-clear.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    Artworks::ArtworkMetadata *artwork = m_TestsApp.getArtwork(0);
    const QStringList &keywords = artwork->getKeywords();

    QStringList expectedKeywords = QString("picture,seagull,bird").split(',');

    VERIFY(expectedKeywords == keywords, "Keywords are not the same!");
    VERIFY(artwork->getDescription() == "Seagull description", "Description is not the same!");
    VERIFY(artwork->getTitle() == "Seagull title", "Title is not the same!");

    artwork->setIsSelected(true);
    m_TestsApp.getFilteredArtworksModel().removeMetadataInSelected();

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForExport(waiter);

    m_TestsApp.dispatch(QMLExtensions::UICommandID::SaveSelected);

    VERIFY(waiter.wait(20), "Timeout exceeded for writing metadata.");
    VERIFY(m_TestsApp.checkExportSucceeded(), "Failed to export");

    m_TestsApp.deleteArtworks(Helpers::IndicesRanges(files.size()));

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    artwork = m_TestsApp.getArtwork(0);

    VERIFY(artwork->getBasicModel()->isDescriptionEmpty(), "Description was not empty");
    VERIFY(artwork->getBasicModel()->isTitleEmpty(), "Title was not empty");
    VERIFY(artwork->getBasicModel()->areKeywordsEmpty(), "Keywords were not empty");

    return 0;
}


