#include "spellingproduceswarningstest.h"
#include <QUrl>
#include <QList>
#include "signalwaiter.h"
#include "xpikstestsapp.h"
#include "testshelpers.h"

QString SpellingProducesWarningsTest::testName() {
    return QLatin1String("SpellingProducesWarningsTest");
}

void SpellingProducesWarningsTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int SpellingProducesWarningsTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto artwork = m_TestsApp.getArtwork(0);
    VERIFY(artwork->isInitialized(), "Artwork is not initialized after import");

    sleepWaitUntil(3, [artwork]() {
        return !Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInTitle) &&
                !Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInDescription) &&
                !Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInKeywords);
    });

    VERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInTitle), "Error for reading title");
    VERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInDescription), "Error for reading description");
    VERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInKeywords), "Error for reading keywords");

    SignalWaiter spellingWaiter;
    m_TestsApp.connectWaiterForSpellcheck(spellingWaiter);

    QString wrongWord = "abbreviatioe";
    artwork->setDescription(artwork->getDescription() + ' ' + wrongWord);
    artwork->setTitle(artwork->getTitle() + ' ' + wrongWord);
    artwork->appendKeyword("correct part " + wrongWord);

    VERIFY(spellingWaiter.wait(5), "Timeout for waiting for first spellcheck results");

    LOG_INFO << "Spellchecking finished. Waiting for warnings...";

    sleepWaitUntil(5, [=]() {
        return Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInTitle) &&
                Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInDescription) &&
                Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInKeywords);
    });

    auto &keywordsModel = artwork->getBasicMetadataModel();

    VERIFY(keywordsModel.hasDescriptionSpellError(), "Description spell error not detected");
    VERIFY(keywordsModel.hasTitleSpellError(), "Title spell error not detected");
    VERIFY(keywordsModel.hasKeywordsSpellError(), "Keywords spell error not detected");

    VERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInTitle),
           "Warning was not produced for title spelling error");
    VERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInDescription),
           "Warning was not produced for description spelling error");
    VERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInKeywords),
           "Warning was not produced for keywords spelling error");

    return 0;
}
