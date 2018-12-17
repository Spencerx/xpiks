#include "zipartworkstest.h"

#include <memory>

#include <QFileInfo>
#include <QLatin1String>
#include <QList>
#include <QObject>
#include <QUrl>
#include <QSignalSpy>
#include <QtGlobal>

#include "Artworks/artworkmetadata.h"
#include "Artworks/imageartwork.h"
#include "Helpers/filehelpers.h"
#include "Models/Connectivity/ziparchiver.h"
#include "Models/settingsmodel.h"
#include "QMLExtensions/uicommandid.h"

#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"

QString ZipArtworksTest::testName() {
    return QLatin1String("ZipArtworksTest");
}

void ZipArtworksTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(true);
}

int ZipArtworksTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg", true);
    files << setupFilePathForTest("images-for-tests/vector/027.jpg", true);
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg", true);

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    m_TestsApp.selectAllArtworks();
    m_TestsApp.dispatch(QMLExtensions::UICommandID::SetupCreatingArchives);

    Models::ZipArchiver &zipArchiver = m_TestsApp.getZipArchiver();
    VERIFY(zipArchiver.getItemsCount() == 2, "ZipArchiver didn't get all the files");

    QSignalSpy progressSpy(&zipArchiver, &Models::ZipArchiver::percentChanged);
    QSignalSpy finishedSpy(&zipArchiver, &Models::ZipArchiver::finishedProcessing);

    m_TestsApp.dispatch(QMLExtensions::UICommandID::CreateArchives, QVariant::fromValue(true));

    sleepWaitUntil(3, [&zipArchiver](){ return zipArchiver.getInProgress(); });
    VERIFY(zipArchiver.getInProgress(), "Archiver is not in progress");

    sleepWaitUntil(20, [&zipArchiver](){ return !zipArchiver.getInProgress(); });
    VERIFY(!zipArchiver.getInProgress(), "Archiver is still in progress");

    VERIFY(!zipArchiver.getHasErrors(), "Errors while zipping");
    VERIFY(progressSpy.count() > 2, "Intermediate progress was not reported");
    VERIFY(finishedSpy.count() == 1, "Archivation wasn't finished");

    for (int i = 0; i < files.length(); ++i) {
        auto artwork = m_TestsApp.getArtwork(i);
        auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
        Q_ASSERT(image != nullptr);

        if (image->hasVectorAttached()) {
            QString zipPath = Helpers::getArchivePath(artwork->getFilepath());
            VERIFY(QFileInfo(zipPath).exists(), "Zip file not found");
        }
    }

    return 0;
}
