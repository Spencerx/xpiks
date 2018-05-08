#include "autodetachvectortest.h"
#include <QUrl>
#include <QFileInfo>
#include <QStringList>
#include <QFile>
#include <QDebug>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "../../xpiks-qt/Commands/commandmanager.h"
#include "../../xpiks-qt/Models/artitemsmodel.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/Models/artworkmetadata.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/Models/imageartwork.h"

QString AutoDetachVectorTest::testName() {
    return QLatin1String("AutoDetachVectorTest");
}

void AutoDetachVectorTest::setup() {
    Models::SettingsModel *settingsModel = m_CommandManager->getSettingsModel();
    settingsModel->setAutoFindVectors(true);

    // copy file
    setupFilePathForTest("images-for-tests/items-to-remove/026.eps");
}

int AutoDetachVectorTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_CommandManager->getArtItemsModel();
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/items-to-remove/026.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_CommandManager->getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));    

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add files");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    Models::ArtworkMetadata *metadata = artItemsModel->getArtwork(0);
    Models::ImageArtwork *image = dynamic_cast<Models::ImageArtwork *>(metadata);
    VERIFY(image != NULL && image->hasVectorAttached(), "Vector is not attached!");

    const QString vectorPath = image->getAttachedVectorPath();
    QFile vectorFile(vectorPath);

    VERIFY(vectorFile.exists(), "Vector can't be found on disk");
    VERIFY(vectorFile.remove(), "Failed to remove vector");
    qDebug() << "Removed vector file" << vectorPath;

    SignalWaiter unavailableVectorsWaiter;
    QObject::connect(artItemsModel, SIGNAL(unavailableVectorsFound()),
                     &unavailableVectorsWaiter, SIGNAL(finished()));

    VERIFY(unavailableVectorsWaiter.wait(5), "Timeout exceeded for unavailable vectors");

    VERIFY(!image->hasVectorAttached(), "Vector should not be attached!");

    return 0;
}


