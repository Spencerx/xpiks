#include "userdictedittest.h"
#include <QUrl>
#include <QList>
#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"

QString UserDictEditTest::testName() {
    return QLatin1String("UserDictEditTest");
}

void UserDictEditTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int UserDictEditTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    Artworks::ArtworkMetadata *metadata = m_TestsApp.getArtwork(0);
    auto &quickBuffer = m_TestsApp.getQuickBuffer();

    // wait for after-add spellchecking
    QThread::sleep(1);

    auto *basicKeywordsModel = metadata->getBasicModel();

    QString wrongWord = "Abbreviatioe";
    metadata->setDescription(metadata->getDescription() + ' ' + wrongWord);
    metadata->setTitle(metadata->getTitle() + ' ' + wrongWord);
    metadata->appendKeyword("correct part " + wrongWord);
    metadata->setIsSelected(true);

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForSpellcheck(waiter);

    m_TestsApp.getFilteredArtworksModel().copyToQuickBuffer(0);

    VERIFY(waiter.wait(5), "Timeout for waiting for spellcheck results");

    // wait for finding suggestions
    QThread::sleep(1);

    VERIFY(basicKeywordsModel->hasDescriptionSpellError(), "Description spell error not detected");
    VERIFY(basicKeywordsModel->hasTitleSpellError(), "Title spell error not detected");
    VERIFY(basicKeywordsModel->hasKeywordsSpellError(), "Keywords spell error not detected");
    VERIFY(quickBuffer.hasSpellErrors(), "Quick Buffer does not contain spelling erros");

    auto &userDictEditModel = m_TestsApp.getUserDictEditModel();
    userDictEditModel.appendKeyword(wrongWord);
    userDictEditModel.saveUserDict();

    sleepWaitUntil(5, [=]() {
        return !basicKeywordsModel->hasDescriptionSpellError() &&
                !basicKeywordsModel->hasTitleSpellError() &&
                !basicKeywordsModel->hasKeywordsSpellError() &&
                !quickBuffer.hasSpellErrors();
    });

    VERIFY(!basicKeywordsModel->hasDescriptionSpellError(), "After adding word. Description spell error is still present");
    VERIFY(!basicKeywordsModel->hasTitleSpellError(), "After adding word. Title spell error is still present");
    VERIFY(!basicKeywordsModel->hasKeywordsSpellError(), "After adding word. Keywords spell error is still present");

    VERIFY(!quickBuffer.hasSpellErrors(), "After adding word. Quick Buffer contains spelling errors");

    return 0;
}


