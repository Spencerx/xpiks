#include "plaintextedittest.h"
#include <QUrl>
#include <QThread>
#include <QList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"

QString PlainTextEditTest::testName() {
    return QLatin1String("PlainTextEditTest");
}

void PlainTextEditTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int PlainTextEditTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForSpellcheck(waiter);

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    // wait for after-add spellchecking
    VERIFY(waiter.wait(5), "Timeout for waiting for initial spellchecks");

    m_TestsApp.selectAllArtworks();
    m_TestsApp.dispatch(QMLExtensions::UICommandID::EditSelectedArtworks);

    Models::CombinedArtworksModel &combinedModel = m_TestsApp.getCombinedArtworksModel();
    auto *basicModel = combinedModel.retrieveBasicMetadataModel();
    VERIFY(!basicModel->hasKeywordsSpellError(), "Should not have errors initially");

    combinedModel.plainTextEdit("test, keyword, abbreviatoe");

    sleepWaitUntil(5, [&]() {
        return (!m_TestsApp.getSpellCheckService().isBusy()) && basicModel->hasKeywordsSpellError();
    });

    VERIFY(basicModel->hasKeywordsSpellError(), "Keywords spell error not detected");

    return 0;
}
