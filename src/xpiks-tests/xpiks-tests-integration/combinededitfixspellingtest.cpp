#include "combinededitfixspellingtest.h"
#include <QUrl>
#include <QThread>
#include <QList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString CombinedEditFixSpellingTest::testName() {
    return QLatin1String("CombinedEditFixSpellingTest");
}

void CombinedEditFixSpellingTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int CombinedEditFixSpellingTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    // wait for after-add spellchecking
    QThread::sleep(1);

    QString wrongWord = "abbreviatioe";

    auto *artwork = m_TestsApp.getArtwork(0);
    Artworks::BasicMetadataModel *basicModel = artwork->getBasicModel();
    SignalWaiter waiter;
    QObject::connect(basicModel, &Artworks::BasicMetadataModel::descriptionSpellingChanged,
                     &waiter, &SignalWaiter::finished);

    QString nextDescription = artwork->getDescription() + ' ' + wrongWord;
    artwork->setDescription(nextDescription);

    VERIFY(waiter.wait(5), "Timeout for waiting for initial spellcheck results");

    // wait for finding suggestions
    QThread::sleep(1);

    VERIFY(basicModel->hasDescriptionSpellError(), "Description spell error not detected");

    artwork->setIsSelected(true);
    m_TestsApp.dispatch(QMLExtensions::UICommandID::EditSelectedArtworks);

    // wait for finding suggestions
    QThread::sleep(1);

    m_TestsApp.dispatch(QMLExtensions::UICommandID::FixSpellingCombined);

    SpellCheck::SpellCheckSuggestionModel &spellSuggestor = m_TestsApp.getSpellSuggestionsModel();
    int rowCount = spellSuggestor.rowCount();
    VERIFY(rowCount > 0, "Spell suggestions are not set");

    auto *combinedKeywordsModel = m_TestsApp.getCombinedArtworksModel().retrieveBasicMetadataModel();
    VERIFY(combinedKeywordsModel->hasDescriptionSpellError(), "Description spell error was not propagated");

    SignalWaiter combinedEditWaiter;
    QObject::connect(combinedKeywordsModel, &Artworks::BasicMetadataModel::descriptionSpellingChanged,
                     &combinedEditWaiter, &SignalWaiter::finished);

    VERIFY(m_TestsApp.selectSpellSuggestions(0), "Cannot select spell suggestions");
    m_TestsApp.getSpellSuggestionsModel().submitCorrections();

    VERIFY(combinedEditWaiter.wait(5), "Timeout for waiting for corrected spellcheck results after replace");

    VERIFY(!combinedKeywordsModel->hasDescriptionSpellError(), "Description spell error was not fixed");

    QString correctDescription = combinedKeywordsModel->getDescription();
    m_TestsApp.getCombinedArtworksModel().saveEdits();

    VERIFY(basicModel->getDescription() == correctDescription, "Description was not saved");

    return 0;
}
