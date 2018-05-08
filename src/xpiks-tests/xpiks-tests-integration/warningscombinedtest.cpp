#include "warningscombinedtest.h"
#include <QUrl>
#include <QFileInfo>
#include <QStringList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "../../xpiks-qt/Commands/commandmanager.h"
#include "../../xpiks-qt/Models/artitemsmodel.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/Models/artworkmetadata.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/Models/imageartwork.h"
#include "testshelpers.h"

QString WarningsCombinedTest::testName() {
    return QLatin1String("WarningsCombinedTest");
}

void WarningsCombinedTest::setup() {
    Models::SettingsModel *settingsModel = m_CommandManager->getSettingsModel();
    settingsModel->setAutoFindVectors(false);
}

int WarningsCombinedTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_CommandManager->getArtItemsModel();
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/read-only/Nokota_Horses.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_CommandManager->getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add file");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    Models::ArtworkMetadata *artwork = artItemsModel->getArtwork(0);
    VERIFY(artwork->isInitialized(), "Artwork is not initialized after import");

    sleepWaitUntil(3, [artwork]() {
        return Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords) &&
                Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty) &&
                Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty);
    });

    VERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords), "Empty keywords not detected");
    VERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty), "Empty description not detected");
    VERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty), "Empty title not detected");

    artwork->setDescription("description");
    artwork->setTitle("title");
    artwork->appendKeyword("correct");

    LOG_INFO << "Waiting for warnings...";

    sleepWaitUntil(5, [artwork]() {
        return !Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords) &&
                !Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty) &&
                !Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty);
    });

    VERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords), "Empty keywords flag not updated");
    VERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty), "Empty description flag not updated");
    VERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty), "Empty title flag not updated");

    return 0;
}
