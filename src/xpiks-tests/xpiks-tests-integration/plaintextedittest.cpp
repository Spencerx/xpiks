#include "plaintextedittest.h"
#include <QUrl>
#include <QFileInfo>
#include <QThread>
#include <QStringList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "testshelpers.h"
#include "../../xpiks-qt/Commands/commandmanager.h"
#include "../../xpiks-qt/Models/artitemsmodel.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/Models/artworkmetadata.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/Models/filteredartitemsproxymodel.h"
#include "../../xpiks-qt/SpellCheck/spellcheckerservice.h"
#include "../../xpiks-qt/Models/combinedartworksmodel.h"
#include "../../xpiks-qt/Common/basickeywordsmodel.h"

QString PlainTextEditTest::testName() {
    return QLatin1String("PlainTextEditTest");
}

void PlainTextEditTest::setup() {
    Models::SettingsModel *settingsModel = m_TestsApp.getSettingsModel();
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int PlainTextEditTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_TestsApp.getArtItemsModel();
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_TestsApp.getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));    

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add file");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    SpellCheck::SpellCheckerService *spellCheckService = m_TestsApp.getSpellCheckerService();
    QObject::connect(spellCheckService, SIGNAL(spellCheckQueueIsEmpty()), &waiter, SIGNAL(finished()));

    Models::FilteredArtItemsProxyModel *filteredModel = m_TestsApp.getFilteredArtItemsModel();
    filteredModel->selectFilteredArtworks();
    filteredModel->combineSelectedArtworks();

    // wait for after-add spellchecking
    VERIFY(waiter.wait(5), "Timeout for waiting for initial spellchecks");

    Models::CombinedArtworksModel *combinedModel = m_TestsApp.getCombinedArtworksModel();
    auto *basicModel = combinedModel->retrieveBasicMetadataModel();
    VERIFY(!basicModel->hasKeywordsSpellError(), "Should not have errors initially");

    combinedModel->plainTextEdit("test, keyword, abbreviatoe");

    sleepWaitUntil(5, [&]() {
        return (!spellCheckService->isBusy()) && basicModel->hasKeywordsSpellError();
    });

    VERIFY(basicModel->hasKeywordsSpellError(), "Keywords spell error not detected");

    return 0;
}


