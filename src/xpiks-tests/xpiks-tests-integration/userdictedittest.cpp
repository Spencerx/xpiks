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

    auto artwork = m_TestsApp.getArtwork(0);
    Models::QuickBuffer &quickBuffer = m_TestsApp.getQuickBuffer();

    auto &basicModel = artwork->getBasicMetadataModel();

    QString wrongWord = "Abbreviatioe";
    artwork->setDescription(artwork->getDescription() + ' ' + wrongWord);
    artwork->setTitle(artwork->getTitle() + ' ' + wrongWord);
    artwork->appendKeyword("correct part " + wrongWord);
    artwork->setIsSelected(true);

    sleepWaitUntil(5, [&artwork]() { return artwork->getBasicModel().hasKeywordsSpellError(); });

    VERIFY(basicModel.hasDescriptionSpellError(), "Description spell error not detected");
    VERIFY(basicModel.hasTitleSpellError(), "Title spell error not detected");
    VERIFY(basicModel.hasKeywordsSpellError(), "Keywords spell error not detected");

    VERIFY(!quickBuffer.hasSpellErrors(), "Quick Buffer has spelling errors");
    m_TestsApp.getFilteredArtworksModel().copyToQuickBuffer(0);
    sleepWaitUntil(5, [&quickBuffer]() { return quickBuffer.hasSpellErrors(); });
    VERIFY(quickBuffer.hasSpellErrors(), "Quick Buffer does not contain spelling errors");

    auto &userDictEditModel = m_TestsApp.getUserDictEditModel();
    userDictEditModel.appendKeyword(wrongWord);
    userDictEditModel.saveUserDict();

    sleepWaitUntil(5, [&basicModel, &quickBuffer]() {
        return !basicModel.hasDescriptionSpellError() &&
                !basicModel.hasTitleSpellError() &&
                !basicModel.hasKeywordsSpellError() &&
                !quickBuffer.hasSpellErrors();
    });

    VERIFY(!basicModel.hasDescriptionSpellError(), "After adding word. Description spell error is still present");
    VERIFY(!basicModel.hasTitleSpellError(), "After adding word. Title spell error is still present");
    VERIFY(!basicModel.hasKeywordsSpellError(), "After adding word. Keywords spell error is still present");

    VERIFY(!quickBuffer.hasSpellErrors(), "After adding word. Quick Buffer contains spelling errors");

    return 0;
}
