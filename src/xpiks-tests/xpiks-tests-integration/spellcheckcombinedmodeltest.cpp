#include "spellcheckcombinedmodeltest.h"
#include <QUrl>
#include <QList>
#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"
#include "Services/SpellCheck/spellsuggestionsitem.h"

QString SpellCheckCombinedModelTest::testName() {
    return QLatin1String("SpellCheckCombinedModelTest");
}

void SpellCheckCombinedModelTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int SpellCheckCombinedModelTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    m_TestsApp.selectAllArtworks();
    m_TestsApp.dispatch(QMLExtensions::UICommandID::SetupEditSelectedArtworks);

    // wait for after-add spellchecking
    QThread::sleep(1);

    SignalWaiter waiter;
    Models::CombinedArtworksModel &combinedModel = m_TestsApp.getCombinedArtworksModel();
    auto *basicModel = combinedModel.retrieveBasicMetadataModel();
    QObject::connect(basicModel, &Artworks::BasicMetadataModel::keywordsSpellingChanged,
                     &waiter, &SignalWaiter::finished);

    QString wrongWord = "abbreviatioe";
    combinedModel.setDescription(combinedModel.getDescription() + ' ' + wrongWord);
    combinedModel.appendKeyword("correct part " + wrongWord);

    sleepWaitUntil(5, [&basicModel]() {
        return basicModel->hasDescriptionSpellError();
    });

    VERIFY(basicModel->hasDescriptionSpellError(), "Description spell error not detected");
    VERIFY(!basicModel->hasTitleSpellError(), "Title spell error not detected");
    VERIFY(basicModel->hasKeywordsSpellError(), "Keywords spell error not detected");    

    sleepWaitUntil(2, [&]() {
        return !m_TestsApp.getSpellCheckService().suggestCorrections(wrongWord).empty();
    });

    m_TestsApp.dispatch(QMLExtensions::UICommandID::ReviewSpellingCombined);

    SpellCheck::SpellCheckSuggestionModel &spellSuggestor = m_TestsApp.getSpellSuggestionsModel();
    int rowCount = spellSuggestor.rowCount();
    VERIFY(rowCount > 0, "Spell suggestions are not set");

    for (int i = 0; i < rowCount; ++i) {
        SpellCheck::SpellSuggestionsItem *suggestionsItem = spellSuggestor.getItem(i);
        VERIFY(suggestionsItem->rowCount() > 0, "No spelling suggestion suggested");
        suggestionsItem->setReplacementIndex(0);
    }

    spellSuggestor.getActionCommand(true)->execute();

    VERIFY(waiter.wait(5), "Timeout for waiting for corrected spellcheck results");

    VERIFY(!basicModel->hasDescriptionSpellError(), "Description spell error was not fixed");
    VERIFY(!basicModel->hasTitleSpellError(), "Title spell error was not fixed");
    VERIFY(!basicModel->hasKeywordsSpellError(), "Keywords spell error was not fixed");

    return 0;
}
