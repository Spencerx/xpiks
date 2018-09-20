#include "addtouserdictionarytest.h"
#include <QList>
#include <QUrl>
#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"

QString AddToUserDictionaryTest::testName() {
    return QLatin1String("AddToUserDictionaryTest");
}

void AddToUserDictionaryTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int AddToUserDictionaryTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");
    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto artwork = m_TestsApp.getArtwork(0);

    auto &basicModel = artwork->getBasicMetadataModel();

    QString wrongWord = "abbreviatioe";
    artwork->setDescription(artwork->getDescription() + ' ' + wrongWord);
    artwork->setTitle(artwork->getTitle() + ' ' + wrongWord);
    artwork->appendKeyword("correct part " + wrongWord);

    sleepWaitUntil(5, [&artwork]() { return artwork->getBasicModel().hasKeywordsSpellError(); });

    VERIFY(basicModel.hasDescriptionSpellError(), "Description spell error not detected");
    VERIFY(basicModel.hasTitleSpellError(), "Title spell error not detected");
    VERIFY(basicModel.hasKeywordsSpellError(), "Keywords spell error not detected");

    m_TestsApp.getUserDictionary().addWord(wrongWord);

    sleepWaitUntil(5, [&basicModel]() {
        return !basicModel.hasDescriptionSpellError() &&
                !basicModel.hasTitleSpellError() &&
                !basicModel.hasKeywordsSpellError();
    });

    int userDictWords = m_TestsApp.getUserDictionary().getWordsCount();
    LOG_DEBUG << "User dict words count:" << userDictWords;

    VERIFY(userDictWords == 1, "Wrong number of words in user dictionary");
    VERIFY(!basicModel.hasDescriptionSpellError(), "After adding word. Description spell error is still present");
    VERIFY(!basicModel.hasTitleSpellError(), "After adding word. Title spell error is still present");
    VERIFY(!basicModel.hasKeywordsSpellError(), "After adding word. Keywords spell error is still present");

    return 0;
}
