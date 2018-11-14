#include "combinededitfixspellingtest.h"

#include <memory>

#include <QLatin1String>
#include <QList>
#include <QUrl>

#include "Artworks/artworkmetadata.h"
#include "Artworks/basicmetadatamodel.h"
#include "Commands/Base/icommand.h"
#include "Models/Editing/combinedartworksmodel.h"
#include "Models/settingsmodel.h"
#include "QMLExtensions/uicommandid.h"
#include "Services/SpellCheck/spellchecksuggestionmodel.h"

#include "testshelpers.h"
#include "xpikstestsapp.h"

QString CombinedEditFixSpellingTest::testName() {
    return QLatin1String("CombinedEditFixSpellingTest");
}

void CombinedEditFixSpellingTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int CombinedEditFixSpellingTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    QString wrongWord = "abbreviatioe";

    auto artwork = m_TestsApp.getArtwork(0);
    auto &basicModel = artwork->getBasicMetadataModel();

    QString nextDescription = artwork->getDescription() + ' ' + wrongWord;
    artwork->setDescription(nextDescription);

    sleepWaitUntil(5, [&]() { return basicModel.hasDescriptionSpellError(); });
    VERIFY(basicModel.hasDescriptionSpellError(), "Description spell error not detected");

    artwork->setIsSelected(true);
    m_TestsApp.dispatch(QMLExtensions::UICommandID::SetupEditSelectedArtworks);

    auto *combinedKeywordsModel = m_TestsApp.getCombinedArtworksModel().retrieveBasicMetadataModel();

    sleepWaitUntil(5, [&]() { return combinedKeywordsModel->hasDescriptionSpellError(); });
    VERIFY(combinedKeywordsModel->hasDescriptionSpellError(), "Description spell error was not propagated");

    m_TestsApp.dispatch(QMLExtensions::UICommandID::ReviewSpellingCombined);

    SpellCheck::SpellCheckSuggestionModel &spellSuggestor = m_TestsApp.getSpellSuggestionsModel();
    int rowCount = spellSuggestor.rowCount();
    VERIFY(rowCount > 0, "Spell suggestions are set");

    VERIFY(m_TestsApp.selectSpellSuggestions(0), "Cannot select spell suggestions");
    m_TestsApp.getSpellSuggestionsModel().getActionCommand(true)->execute();

    sleepWaitUntil(5, [&]() { return !combinedKeywordsModel->hasDescriptionSpellError(); });
    VERIFY(!combinedKeywordsModel->hasDescriptionSpellError(), "Description spell error was not fixed after replace");

    QString correctDescription = combinedKeywordsModel->getDescription();
    m_TestsApp.getCombinedArtworksModel().getActionCommand(true)->execute();

    VERIFY(basicModel.getDescription() == correctDescription, "Description was not saved");

    return 0;
}
