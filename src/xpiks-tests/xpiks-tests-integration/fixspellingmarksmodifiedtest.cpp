#include "fixspellingmarksmodifiedtest.h"

#include <memory>

#include <QLatin1String>
#include <QList>
#include <QObject>
#include <QStringList>
#include <QUrl>

#include "Artworks/artworkmetadata.h"
#include "Artworks/basickeywordsmodel.h"
#include "Commands/Base/icommand.h"
#include "Models/settingsmodel.h"
#include "QMLExtensions/uicommandid.h"
#include "Services/SpellCheck/spellcheckservice.h"
#include "Services/SpellCheck/spellchecksuggestionmodel.h"

#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"

QString FixSpellingMarksModifiedTest::testName() {
    return QLatin1String("FixSpellingMarksModifiedTest");
}

void FixSpellingMarksModifiedTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int FixSpellingMarksModifiedTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto artwork = m_TestsApp.getArtwork(0);
    auto &basicModel = artwork->getBasicModel();

    SignalWaiter waiter;
    QObject::connect(&basicModel, &Artworks::BasicKeywordsModel::keywordsSpellingChanged,
                     &waiter, &SignalWaiter::finished);

    QString wrongWord = "abbreviatioe";
    artwork->appendKeyword(wrongWord);

    VERIFY(waiter.wait(5), "Timeout for waiting for initial spellcheck results");

    sleepWaitUntil(5, [&basicModel]() { return basicModel.hasKeywordsSpellError(); });
    VERIFY(basicModel.hasKeywordsSpellError(), "Keywords spell error not detected");    
    sleepWaitUntil(2, [&]() {
        return !m_TestsApp.getSpellCheckService().suggestCorrections(wrongWord).empty();
    });

    artwork->setIsSelected(true);
    m_TestsApp.dispatch(QMLExtensions::UICommandID::ReviewSpellingInSelected);

    VERIFY(m_TestsApp.getSpellSuggestionsModel().rowCount() > 0, "Spell suggestions are not set");
    VERIFY(m_TestsApp.selectSpellSuggestions(0), "Failed to select spell suggestions");

    m_TestsApp.getSpellSuggestionsModel().getActionCommand(true)->execute();

    VERIFY(waiter.wait(5), "Timeout for waiting for corrected spellcheck results");

    VERIFY(!basicModel.hasKeywordsSpellError(), "Keywords spell error was not fixed");
    VERIFY(m_TestsApp.getArtwork(0)->isModified(), "Artwork was not modified");

    return 0;
}
