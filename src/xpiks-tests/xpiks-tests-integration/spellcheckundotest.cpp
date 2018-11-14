#include "spellcheckundotest.h"

#include <memory>

#include <QDebug>
#include <QLatin1String>
#include <QList>
#include <QThread>
#include <QUrl>
#include <QtGlobal>

#include "Artworks/artworkmetadata.h"
#include "Artworks/basickeywordsmodel.h"
#include "Artworks/basicmetadatamodel.h"
#include "Models/Artworks/filteredartworkslistmodel.h"
#include "Models/settingsmodel.h"

#include "signalwaiter.h"
#include "testshelpers.h"
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

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForSpellcheck(waiter);

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");
    VERIFY(waiter.wait(5), "Timeout for waiting for first spellcheck results");

    auto artwork = m_TestsApp.getArtwork(0);

    QString wrongWord = "abbreviatioe";
    artwork->setDescription(artwork->getDescription() + ' ' + wrongWord);
    artwork->setTitle(artwork->getTitle() + ' ' + wrongWord);
    artwork->appendKeyword("correct part " + wrongWord);
    artwork->setIsSelected(true);

    sleepWaitUntil(5, [&artwork]() {
        return artwork->getBasicModel().hasKeywordsSpellError();
    });

    // wait for finding suggestions
    QThread::sleep(1);

    auto &basicModel = artwork->getBasicMetadataModel();

    CHECK_HAS_ERRORS_EVERYWHERE(basicModel);

    m_TestsApp.getFilteredArtworksModel().clearKeywords(0);
    QThread::sleep(1);

    VERIFY(!basicModel.hasKeywordsSpellError(), "Keywords spell error not cleared");
    VERIFY(m_TestsApp.undoLastAction(), "Failed to undo last action");

    qDebug() << "Checking second time...";
    sleepWaitUntil(5, [&artwork]() {
        return artwork->getBasicModel().hasKeywordsSpellError();
    });

    CHECK_HAS_ERRORS_EVERYWHERE(basicModel);

    return 0;
}
