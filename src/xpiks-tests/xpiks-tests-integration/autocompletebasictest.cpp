#include "autocompletebasictest.h"
#include <QUrl>
#include <QList>
#include <QDebug>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString AutoCompleteBasicTest::testName() {
    return QLatin1String("AutoCompleteBasicTest");
}

void AutoCompleteBasicTest::setup() {
    m_TestsApp.getSettingsModel().setUseKeywordsAutoComplete(true);
}

int AutoCompleteBasicTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto artwork = m_TestsApp.getArtwork(0);
    auto &basicModel = artwork->getBasicMetadataModel();

    AutoComplete::AutoCompleteService &acService = m_TestsApp.getAutoCompleteService();
    AutoComplete::KeywordsAutoCompleteModel &acModel = m_TestsApp.getKeywordsAutoCompleteModel();
    AutoComplete::KeywordsCompletionsModel &completionsModel = acModel.getCompletionsSource();

    SignalWaiter completionWaiter;
    QObject::connect(&basicModel, &Artworks::BasicMetadataModel::completionsAvailable,
                     &completionWaiter, &SignalWaiter::finished);

    VERIFY(acModel.getCount() == 0, "AC model was not empty");

    // --------------------------------------------------------------

    acService.generateCompletions("tes", &basicModel);

    VERIFY(completionWaiter.wait(10), "Timeout while waiting for the completion");

    acModel.initializeCompletions();

    qInfo() << "Generated" << acModel.getCount() << "completions";
    qInfo() << "Completions:" << completionsModel.getLastGeneratedCompletions();

    VERIFY(acModel.getCount() > 0, "AC model didn't receive the completions");
    VERIFY(completionsModel.containsWord("test"), "AC model has irrelevant results");

    acModel.clear();

    // --------------------------------------------------------------

    acService.generateCompletions("Tes", &basicModel);

    VERIFY(completionWaiter.wait(10), "Timeout while waiting for the completion");

    acModel.initializeCompletions();

    qInfo() << "Generated" << acModel.getCount() << "completions";
    qInfo() << "Completions:" << completionsModel.getLastGeneratedCompletions();

    VERIFY(acModel.getCount() > 0, "AC model didn't receive the completions second time");
    VERIFY(completionsModel.containsWord("test"), "AC model has irrelevant results");

    // --------------------------------------------------------------

    VERIFY(acModel.moveSelectionDown(), "AC model can't move selection down");
    // in the beginning the selection index is -1
    VERIFY(acModel.moveSelectionDown(), "AC model can't move selection down");
    VERIFY(acModel.moveSelectionUp(), "AC model can't move selection back up");
    VERIFY(!acModel.moveSelectionUp(), "AC model can move selection back up while being at a top");

    return 0;
}
