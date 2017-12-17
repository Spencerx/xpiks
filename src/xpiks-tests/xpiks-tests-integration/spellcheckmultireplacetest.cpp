#include "spellcheckmultireplacetest.h"
#include <QUrl>
#include <QFileInfo>
#include <QThread>
#include <QStringList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "../../xpiks-qt/Commands/commandmanager.h"
#include "../../xpiks-qt/Models/artitemsmodel.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/Models/artworkmetadata.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/Models/filteredartitemsproxymodel.h"
#include "../../xpiks-qt/SpellCheck/spellchecksuggestionmodel.h"
#include "../../xpiks-qt/SpellCheck/spellsuggestionsitem.h"
#include "../../xpiks-qt/SpellCheck/spellcheckerservice.h"
#include "testshelpers.h"

QString SpellCheckMultireplaceTest::testName() {
    return QLatin1String("SpellCheckMultireplaceTest");
}

void SpellCheckMultireplaceTest::setup() {
    Models::SettingsModel *settingsModel = m_CommandManager->getSettingsModel();
    settingsModel->setUseSpellCheck(true);
}

int SpellCheckMultireplaceTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_CommandManager->getArtItemsModel();
    QList<QUrl> files;
    files << getFilePathForTest("images-for-tests/vector/026.jpg");
    files << getFilePathForTest("images-for-tests/vector/027.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_CommandManager->getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add file");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    Models::ArtworkMetadata *metadata = artItemsModel->getArtwork(0);

    QString wrongWord = "abbreviatioe";
    metadata->setDescription(metadata->getDescription() + ' ' + wrongWord);
    metadata->setTitle(metadata->getTitle() + ' ' + wrongWord);
    metadata->appendKeyword("correct part " + wrongWord);
    metadata->setIsSelected(true);

    // wait for after-add spellchecking
    QThread::sleep(1);

    Models::FilteredArtItemsProxyModel *filteredModel = m_CommandManager->getFilteredArtItemsModel();
    SpellCheck::SpellCheckerService *spellCheckService = m_CommandManager->getSpellCheckerService();
    QObject::connect(spellCheckService, SIGNAL(spellCheckQueueIsEmpty()), &waiter, SIGNAL(finished()));

    filteredModel->spellCheckSelected();

    VERIFY(waiter.wait(5), "Timeout for waiting for spellcheck results");

    // wait for finding suggestions
    QThread::sleep(1);

    auto *basicKeywordsModel = metadata->getBasicModel();

    VERIFY(basicKeywordsModel->hasDescriptionSpellError(), "Description spell error not detected");
    VERIFY(basicKeywordsModel->hasTitleSpellError(), "Title spell error not detected");
    VERIFY(basicKeywordsModel->hasKeywordsSpellError(), "Keywords spell error not detected");

    sleepWaitUntil(5, [=]() {
        return spellCheckService->getSuggestionsCount() > 0;
    });

    artItemsModel->suggestCorrections(0);

    SpellCheck::SpellCheckSuggestionModel *spellSuggestor = m_CommandManager->getSpellSuggestionsModel();
    int rowCount = spellSuggestor->rowCount();
    VERIFY(rowCount > 0, "Spell suggestions are not set");

    for (int i = 0; i < rowCount; ++i) {
        SpellCheck::SpellSuggestionsItem *suggestionsItem = spellSuggestor->getItem(i);
        VERIFY(suggestionsItem->rowCount() > 0, "No spelling suggestion suggested");
        suggestionsItem->setReplacementIndex(0);
    }

    spellSuggestor->submitCorrections();

    VERIFY(waiter.wait(5), "Timeout for waiting for corrected spellcheck results");

    VERIFY(!basicKeywordsModel->hasDescriptionSpellError(), "Description spell error was not fixed");
    VERIFY(!basicKeywordsModel->hasTitleSpellError(), "Title spell error was not fixed");
    VERIFY(!basicKeywordsModel->hasKeywordsSpellError(), "Keywords spell error was not fixed");

    return 0;
}


