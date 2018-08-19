#include "spellcheckundotest.h"
#include <QUrl>
#include <QList>
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString SpellCheckUndoTest::testName() {
    return QLatin1String("SpellCheckUndoTest");
}

void SpellCheckUndoTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

#define CHECK_HAS_ERRORS_EVERYWHERE(basicModel)\
    VERIFY(basicModel.hasDescriptionSpellError(), "Description spell error not detected");\
    VERIFY(basicModel.hasTitleSpellError(), "Title spell error not detected");\
    VERIFY(basicModel.hasKeywordsSpellError(), "Keywords spell error not detected")

int SpellCheckUndoTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    // wait for after-add spellchecking
    QThread::sleep(1);

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForSpellcheck(waiter);
    auto artwork = m_TestsApp.getArtwork(0);

    QString wrongWord = "abbreviatioe";
    artwork->setDescription(artwork->getDescription() + ' ' + wrongWord);
    artwork->setTitle(artwork->getTitle() + ' ' + wrongWord);
    artwork->appendKeyword("correct part " + wrongWord);
    artwork->setIsSelected(true);

    VERIFY(waiter.wait(5), "Timeout for waiting for first spellcheck results");

    // wait for finding suggestions
    QThread::sleep(1);
    auto &basicModel = artwork->getBasicMetadataModel();

    CHECK_HAS_ERRORS_EVERYWHERE(basicModel);

    m_TestsApp.getFilteredArtworksModel().clearKeywords(0);
    QThread::sleep(1);

    VERIFY(!basicModel.hasKeywordsSpellError(), "Keywords spell error not cleared");
    VERIFY(m_TestsApp.undoLastAction(), "Failed to undo last action");
    VERIFY(waiter.wait(5), "Timeout for waiting for second spellcheck results");

    // wait for finding suggestions
    QThread::sleep(1);

    qDebug() << "Checking second time...";
    CHECK_HAS_ERRORS_EVERYWHERE(basicModel);

    return 0;
}




