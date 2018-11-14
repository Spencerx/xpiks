#include "locallibrarysearchtest.h"

#include <QLatin1String>
#include <QList>
#include <QUrl>
#include <QVariant>

#include "Models/settingsmodel.h"
#include "QMLExtensions/uicommandid.h"
#include "Suggestion/keywordssuggestor.h"

#include "testshelpers.h"
#include "xpikstestsapp.h"

QString LocalLibrarySearchTest::testName() {
    return QLatin1String("LocalLibrarySearchTest");
}

void LocalLibrarySearchTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int LocalLibrarySearchTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    m_TestsApp.dispatch(QMLExtensions::UICommandID::InitSuggestionArtwork, QVariant(0));

    Suggestion::KeywordsSuggestor &suggestor = m_TestsApp.getKeywordsSuggestor();

    suggestor.setSelectedSourceIndex(suggestor.getEnginesCount() - 1); // local search is the last one
    VERIFY(suggestor.getIsLocalSearch(), "Local search cannot be chosen");

    // 0 was meant to be "any search result"
    suggestor.searchArtworks("abstract", 0);
    VERIFY(suggestor.getIsInProgress(), "Keywords suggestor did not start");

    sleepWaitUntil(5, [&suggestor]() {
        return suggestor.getIsInProgress() == false;
    });

    VERIFY(suggestor.getIsInProgress() == false, "Keywords suggestor is still working");
    VERIFY(suggestor.rowCount() >= 2, "Artworks cannot be found");

    return 0;
}
