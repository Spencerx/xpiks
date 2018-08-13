#include "fixspellingmarksmodifiedtest.h"
#include <QUrl>
#include <QThread>
#include <QList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"

QString FixSpellingMarksModifiedTest::testName() {
    return QLatin1String("FixSpellingMarksModifiedTest");
}

void FixSpellingMarksModifiedTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int FixSpellingMarksModifiedTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    Artworks::ArtworkMetadata *artwork = m_TestsApp.getArtwork(0);
    Artworks::BasicMetadataModel *basicModel = artwork->getBasicModel();

    SignalWaiter waiter;
    QObject::connect(basicModel, &Artworks::BasicMetadataModel::keywordsSpellingChanged,
                     &waiter, &SignalWaiter::finished);

    QString wrongWord = "abbreviatioe";
    artwork->appendKeyword(wrongWord);

    VERIFY(waiter.wait(5), "Timeout for waiting for initial spellcheck results");

    sleepWaitUntil(5, [&]() { return basicModel->hasKeywordsSpellError(); });
    VERIFY(basicModel->hasKeywordsSpellError(), "Keywords spell error not detected");

    artwork->setIsSelected(true);
    m_TestsApp.dispatch(QMLExtensions::UICommandID::FixSpellingInSelected);

    VERIFY(m_TestsApp.getSpellSuggestionsModel().rowCount() > 0, "Spell suggestions are not set");
    VERIFY(m_TestsApp.selectSpellSuggestions(0), "Failed to select spell suggestions");

    m_TestsApp.getSpellSuggestionsModel().submitCorrections();

    VERIFY(waiter.wait(5), "Timeout for waiting for corrected spellcheck results");

    VERIFY(!basicModel->hasKeywordsSpellError(), "Keywords spell error was not fixed");
    VERIFY(m_TestsApp.getArtwork(0)->isModified(), "Artwork was not modified");

    return 0;
}
