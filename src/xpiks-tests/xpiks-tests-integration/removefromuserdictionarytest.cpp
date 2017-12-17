#include "removefromuserdictionarytest.h"
#include "../../xpiks-qt/Commands/commandmanager.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/Models/artitemsmodel.h"
#include "../../xpiks-qt/SpellCheck/spellcheckerservice.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/Models/filteredartitemsproxymodel.h"
#include "../../xpiks-qt/Models/combinedartworksmodel.h"
#include "../../xpiks-qt/Common/basickeywordsmodel.h"
#include "signalwaiter.h"
#include "testshelpers.h"
#include <QObject>

QString RemoveFromUserDictionaryTest::testName() {
    return QLatin1String("RemoveFromUserDictionaryTest");
}

void RemoveFromUserDictionaryTest::setup() {
    Models::SettingsModel *settingsModel = m_CommandManager->getSettingsModel();
    settingsModel->setUseSpellCheck(true);
}

int RemoveFromUserDictionaryTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_CommandManager->getArtItemsModel();

    QList<QUrl> files;
    files << getFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_CommandManager->getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add file");

    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    Models::ArtworkMetadata *metadata = artItemsModel->getArtwork(0);

    // wait for after-add spellchecking
    QThread::sleep(1);

    auto *basicKeywordsModel = metadata->getBasicModel();

    QString wrongWord = "abbreviatioe";
    metadata->setDescription(metadata->getDescription() + " " + wrongWord);
    metadata->setTitle(metadata->getTitle() + " " + wrongWord);
    metadata->appendKeyword("correct part " + wrongWord);
    metadata->setIsSelected(true);

    Models::FilteredArtItemsProxyModel *filteredModel = m_CommandManager->getFilteredArtItemsModel();
    SpellCheck::SpellCheckerService *spellCheckService = m_CommandManager->getSpellCheckerService();
    QObject::connect(spellCheckService, SIGNAL(spellCheckQueueIsEmpty()), &waiter, SIGNAL(finished()));

    filteredModel->spellCheckSelected();

    VERIFY(waiter.wait(5), "Timeout for waiting for spellcheck results");

    // wait for finding suggestions
    QThread::sleep(1);

    VERIFY(basicKeywordsModel->hasDescriptionSpellError(), "Description spell error not detected");
    VERIFY(basicKeywordsModel->hasTitleSpellError(), "Title spell error not detected");
    VERIFY(basicKeywordsModel->hasKeywordsSpellError(), "Keywords spell error not detected");

    spellCheckService->addWordToUserDictionary(wrongWord);

    SignalWaiter spellingWaiter;
    QObject::connect(spellCheckService, SIGNAL(spellCheckQueueIsEmpty()), &spellingWaiter, SIGNAL(finished()));

    QCoreApplication::processEvents(QEventLoop::AllEvents);

    // wait add user word to finish
    VERIFY(spellingWaiter.wait(5), "Timeout for waiting for spellcheck results");

    sleepWaitUntil(5, [=]() {
        return !basicKeywordsModel->hasDescriptionSpellError() &&
                !basicKeywordsModel->hasTitleSpellError() &&
                !basicKeywordsModel->hasKeywordsSpellError();
    });

    int userDictWords = spellCheckService->getUserDictWordsNumber();

    VERIFY(userDictWords == 1, "Wrong number of words in user dictionary");
    VERIFY(!basicKeywordsModel->hasDescriptionSpellError(), "After adding word. Description spell error is still present");
    VERIFY(!basicKeywordsModel->hasTitleSpellError(), "After adding word. Title spell error is still present");
    VERIFY(!basicKeywordsModel->hasKeywordsSpellError(), "After adding word. Keywords spell error is still present");

    // now clean user dict

    SignalWaiter clearWaiter;
    QObject::connect(spellCheckService, SIGNAL(spellCheckQueueIsEmpty()), &clearWaiter, SIGNAL(finished()));

    spellCheckService->clearUserDictionary();

    // wait clear user dict to finish

    QCoreApplication::processEvents(QEventLoop::AllEvents);

    // wait clear user word to finish
    VERIFY(clearWaiter.wait(5), "Timeout for waiting for spellcheck results");

    sleepWaitUntil(5, [=]() {
        return basicKeywordsModel->hasDescriptionSpellError() &&
                basicKeywordsModel->hasTitleSpellError() &&
                basicKeywordsModel->hasKeywordsSpellError();
    });

    userDictWords = spellCheckService->getUserDictWordsNumber();

    VERIFY(userDictWords == 0, "Wrong number of words in user dictionary");
    VERIFY(basicKeywordsModel->hasDescriptionSpellError(), "Description spell error not detected again");
    VERIFY(basicKeywordsModel->hasTitleSpellError(), "Title spell error not detected again");
    VERIFY(basicKeywordsModel->hasKeywordsSpellError(), "Keywords spell error not detected again");

    return 0;
}
