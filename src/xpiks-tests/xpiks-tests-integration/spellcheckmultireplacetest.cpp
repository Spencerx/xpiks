#include "spellcheckmultireplacetest.h"
#include <QUrl>
#include <QList>
#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"

QString SpellCheckMultireplaceTest::testName() {
    return QLatin1String("SpellCheckMultireplaceTest");
}

void SpellCheckMultireplaceTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int SpellCheckMultireplaceTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");
    files << setupFilePathForTest("images-for-tests/vector/027.jpg");

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForSpellcheck(waiter);

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");
    VERIFY(waiter.wait(5), "Timeout for waiting for initial spellcheck results");

    auto artwork = m_TestsApp.getArtwork(0);

    QString wrongWord = "abbreviatioe";
    artwork->setDescription(artwork->getDescription() + ' ' + wrongWord);
    artwork->setTitle(artwork->getTitle() + ' ' + wrongWord);
    artwork->appendKeyword("correct part " + wrongWord);
    artwork->setIsSelected(true);

    sleepWaitUntil(5, [&artwork]() {
        return artwork->getBasicModel().hasKeywordsSpellError();
    });

    auto &basicMetadataModel = artwork->getBasicMetadataModel();

    VERIFY(basicMetadataModel.hasDescriptionSpellError(), "Description spell error not detected");
    VERIFY(basicMetadataModel.hasTitleSpellError(), "Title spell error not detected");
    VERIFY(basicMetadataModel.hasKeywordsSpellError(), "Keywords spell error not detected");

    sleepWaitUntil(5, [&]() {
        return m_TestsApp.getSpellCheckService().getSuggestionsCount() > 0;
    });

    m_TestsApp.dispatch(QMLExtensions::UICommandID::FixSpellingArtwork, QVariant(0));

    SpellCheck::SpellCheckSuggestionModel &spellSuggestor = m_TestsApp.getSpellSuggestionsModel();
    int rowCount = spellSuggestor.rowCount();
    VERIFY(rowCount > 0, "Spell suggestions are not set");

    for (int i = 0; i < rowCount; ++i) {
        SpellCheck::SpellSuggestionsItem *suggestionsItem = spellSuggestor.getItem(i);
        VERIFY(suggestionsItem->rowCount() > 0, "No spelling suggestion suggested");
        suggestionsItem->setReplacementIndex(0);
    }

    spellSuggestor.submitCorrections();

    VERIFY(waiter.wait(5), "Timeout for waiting for corrected spellcheck results");

    VERIFY(!basicMetadataModel.hasDescriptionSpellError(), "Description spell error was not fixed");
    VERIFY(!basicMetadataModel.hasTitleSpellError(), "Title spell error was not fixed");
    VERIFY(!basicMetadataModel.hasKeywordsSpellError(), "Keywords spell error was not fixed");

    return 0;
}


