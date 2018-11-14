#include "duplicatesearchtest.h"

#include <memory>

#include <QLatin1String>
#include <QList>
#include <QStringList>
#include <QUrl>

#include "Artworks/artworkmetadata.h"
#include "Models/Artworks/filteredartworkslistmodel.h"
#include "Models/settingsmodel.h"

#include "testshelpers.h"
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

    auto artwork = m_TestsApp.getArtwork(0);
    artwork->clearModel();
    m_TestsApp
            .getFilteredArtworksModel()
            .pasteKeywords(0, QStringList() << "cat" << "Mouse" << "mice" << "on");

    sleepWaitUntil(5, [&artwork]() { return artwork->hasDuplicates(0); });

    VERIFY(!artwork->hasDuplicates(0), "Duplicates detected for unique keyword");
    VERIFY(artwork->hasDuplicates(1), "Duplicate not detected for mouse");
    VERIFY(artwork->hasDuplicates(2), "Duplicate not detected for mice");

    artwork->setDescription("on");
    m_TestsApp
            .getFilteredArtworksModel()
            .appendKeyword(0, "cats");

    sleepWaitUntil(5, [&artwork]() { return artwork->hasDuplicates(0); });

    VERIFY(artwork->hasDuplicates(0), "Duplicates not detected singular");
    VERIFY(artwork->hasDuplicates(1), "Duplicate not detected for mouse after append");
    VERIFY(artwork->hasDuplicates(2), "Duplicate not detected for mice after append");
    // cat
    VERIFY(artwork->hasDuplicates(4), "Duplicates not detected for plural");
    // on
    VERIFY(!artwork->hasDuplicates(3), "Duplicates detected for 2 chars word");

    return 0;
}
