#include "duplicatesearchtest.h"
#include "signalwaiter.h"
#include "testshelpers.h"
#include <QObject>
#include "xpikstestsapp.h"

QString DuplicateSearchTest::testName() {
    return QLatin1String("DuplicateSearchTest");
}

void DuplicateSearchTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int DuplicateSearchTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/mixed/cat.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForSpellcheck(waiter);

    Artworks::ArtworkMetadata *artwork = m_TestsApp.getArtwork(0);
    artwork->clearModel();
    m_TestsApp
            .getFilteredArtworksModel()
            .pasteKeywords(0, QStringList() << "cat" << "Mouse" << "mice" << "on");

    VERIFY(waiter.wait(5), "Timeout for waiting for first spellcheck results");

    VERIFY(!artwork->hasDuplicates(0), "Duplicates detected for unique keyword");
    VERIFY(artwork->hasDuplicates(1), "Duplicate not detected for mouse");
    VERIFY(artwork->hasDuplicates(2), "Duplicate not detected for mice");

    artwork->setDescription("on");
    m_TestsApp
            .getFilteredArtworksModel()
            .appendKeyword(0, "cats");

    sleepWaitUntil(5, [&artwork]() {
        return artwork->hasDuplicates(0);
    });

    VERIFY(artwork->hasDuplicates(0), "Duplicates not detected singular");
    VERIFY(artwork->hasDuplicates(1), "Duplicate not detected for mouse after append");
    VERIFY(artwork->hasDuplicates(2), "Duplicate not detected for mice after append");
    // cat
    VERIFY(artwork->hasDuplicates(4), "Duplicates not detected for plural");
    // on
    VERIFY(!artwork->hasDuplicates(3), "Duplicates detected for 2 chars word");

    return 0;
}
