#include "userdictedittest.h"
#include "../../xpiks-qt/Commands/commandmanager.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/Models/artitemsmodel.h"
#include "../../xpiks-qt/SpellCheck/spellcheckerservice.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/Models/filteredartitemsproxymodel.h"
#include "../../xpiks-qt/Models/combinedartworksmodel.h"
#include "../../xpiks-qt/Common/basickeywordsmodel.h"
#include "../../xpiks-qt/SpellCheck/userdicteditmodel.h"
#include "../../xpiks-qt/QuickBuffer/quickbuffer.h"
#include "signalwaiter.h"
#include "testshelpers.h"
#include <QObject>

QString UserDictEditTest::testName() {
    return QLatin1String("UserDictEditTest");
}

void UserDictEditTest::setup() {
    // create a memory leak just for this test because of issue with spellchecking
    // object being destroyed before it will be properly released in SpellCheckItem's destructor
    m_UserDictEditModel = new SpellCheck::UserDictEditModel();
    m_UserDictEditModel->setCommandManager(m_CommandManager);
    m_UserDictEditModel->initializeModel();

    Models::SettingsModel *settingsModel = m_TestsApp.getSettingsModel();
    m_TestsApp.getSettingsModel().setUseSpellCheck(true);
}

int UserDictEditTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_TestsApp.getArtItemsModel();

    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_TestsApp.getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add file");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    Artworks::ArtworkMetadata *metadata = m_TestsApp.getArtwork(0);
    auto *quickBuffer = m_TestsApp.getQuickBuffer();

    // wait for after-add spellchecking
    QThread::sleep(1);

    auto *basicKeywordsModel = metadata->getBasicModel();

    QString wrongWord = "Abbreviatioe";
    metadata->setDescription(metadata->getDescription() + ' ' + wrongWord);
    metadata->setTitle(metadata->getTitle() + ' ' + wrongWord);
    metadata->appendKeyword("correct part " + wrongWord);
    metadata->setIsSelected(true);

    Models::FilteredArtItemsProxyModel *filteredModel = m_TestsApp.getFilteredArtItemsModel();
    SpellCheck::SpellCheckerService *spellCheckService = m_TestsApp.getSpellCheckerService();
    QObject::connect(spellCheckService, SIGNAL(spellCheckQueueIsEmpty()), &waiter, SIGNAL(finished()));

    filteredModel->spellCheckSelected();
    filteredModel->copyToQuickBuffer(0);

    VERIFY(waiter.wait(5), "Timeout for waiting for spellcheck results");

    // wait for finding suggestions
    QThread::sleep(1);

    VERIFY(basicKeywordsModel->hasDescriptionSpellError(), "Description spell error not detected");
    VERIFY(basicKeywordsModel->hasTitleSpellError(), "Title spell error not detected");
    VERIFY(basicKeywordsModel->hasKeywordsSpellError(), "Keywords spell error not detected");
    VERIFY(quickBuffer->hasSpellErrors(), "Quick Buffer does not contain spelling erros");

    m_UserDictEditModel->appendKeyword(wrongWord);
    m_UserDictEditModel->saveUserDict();

    sleepWaitUntil(5, [=]() {
        return !basicKeywordsModel->hasDescriptionSpellError() &&
                !basicKeywordsModel->hasTitleSpellError() &&
                !basicKeywordsModel->hasKeywordsSpellError() &&
                !quickBuffer->hasSpellErrors();
    });

    VERIFY(!basicKeywordsModel->hasDescriptionSpellError(), "After adding word. Description spell error is still present");
    VERIFY(!basicKeywordsModel->hasTitleSpellError(), "After adding word. Title spell error is still present");
    VERIFY(!basicKeywordsModel->hasKeywordsSpellError(), "After adding word. Keywords spell error is still present");

    VERIFY(!quickBuffer->hasSpellErrors(), "After adding word. Quick Buffer contains spelling errors");

    return 0;
}


