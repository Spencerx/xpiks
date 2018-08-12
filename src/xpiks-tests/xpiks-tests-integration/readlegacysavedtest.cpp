#include "readlegacysavedtest.h"
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

QString ReadLegacySavedTest::testName() {
    return QLatin1String("ReadLegacySavedTest");
}

void ReadLegacySavedTest::setup() {
    Models::SettingsModel *settingsModel = m_TestsApp.getSettingsModel();
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int ReadLegacySavedTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_TestsApp.getArtItemsModel();
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/img_0007.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_TestsApp.getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add file");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    Artworks::ArtworkMetadata *metadata = m_TestsApp.getArtwork(0);
    const QStringList &keywords = metadata->getKeywords();

    QStringList expectedKeywords = QString("rock,nature,landscape,white,background,beautiful,sun,light,mountain,outdoor,top,rocky,snow,fog,horizon").split(',');

    Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork*>(metadata);

    VERIFY(expectedKeywords == keywords, "Keywords are not the same!");
    VERIFY(image->getImageSize().width() == 2752, "Image width was read incorrectly");
    VERIFY(image->getImageSize().height() == 2794, "Image height was read incorrectly");

    return 0;
}
