#include "addtouserdictionarytest.h"
#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"
#include <QObject>

QString AddToUserDictionaryTest::testName() {
    return QLatin1String("AddToUserDictionaryTest");
}

void AddToUserDictionaryTest::setup() {
    m_TestsApp.setUseSpellCheck(true);
}

int AddToUserDictionaryTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");
    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    Artworks::ArtworkMetadata *artwork = m_TestsApp.getArtwork(0);

    // wait for after-add spellchecking
    QThread::sleep(1);

    auto *basicKeywordsModel = artwork->getBasicModel();

    QString wrongWord = "abbreviatioe";
    artwork->setDescription(artwork->getDescription() + ' ' + wrongWord);
    artwork->setTitle(artwork->getTitle() + ' ' + wrongWord);
    artwork->appendKeyword("correct part " + wrongWord);

    SignalWaiter spellCheckWaiter1;
    m_TestsApp.connectWaiterForSpellcheck(spellCheckWaiter1);

    VERIFY(spellCheckWaiter1.wait(5), "Timeout for waiting for spellcheck results");

    // wait for finding suggestions
    QThread::sleep(1);

    VERIFY(basicKeywordsModel->hasDescriptionSpellError(), "Description spell error not detected");
    VERIFY(basicKeywordsModel->hasTitleSpellError(), "Title spell error not detected");
    VERIFY(basicKeywordsModel->hasKeywordsSpellError(), "Keywords spell error not detected");

    m_TestsApp.getUserDictionary().addWord(wrongWord);

    QCoreApplication::processEvents(QEventLoop::AllEvents);

    sleepWaitUntil(5, [=]() {
        return !basicKeywordsModel->hasDescriptionSpellError() &&
                !basicKeywordsModel->hasTitleSpellError() &&
                !basicKeywordsModel->hasKeywordsSpellError();
    });

    int userDictWords = spellCheckService->getUserDictWordsNumber();
    LOG_DEBUG << "User dict words count:" << userDictWords;

    VERIFY(userDictWords == 1, "Wrong number of words in user dictionary");
    VERIFY(!basicKeywordsModel->hasDescriptionSpellError(), "After adding word. Description spell error is still present");
    VERIFY(!basicKeywordsModel->hasTitleSpellError(), "After adding word. Title spell error is still present");
    VERIFY(!basicKeywordsModel->hasKeywordsSpellError(), "After adding word. Keywords spell error is still present");

    return 0;
}
