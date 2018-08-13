#include "faileduploadstest.h"
#include <QDebug>
#include <QList>
#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"

QString FailedUploadsTest::testName() {
    return QLatin1String("FailedUploadsTest");
}

void FailedUploadsTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int FailedUploadsTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    const QString filepath = m_TestsApp.getArtwork(0)->getFilepath();

    m_TestsApp.selectAllArtworks();
    m_TestsApp.dispatch(QMLExtensions::UICommandID::UploadSelected);

    Models::UploadInfoRepository &uploadRepo = m_TestsApp.getUploadInfoRepository();

    auto remote1 = uploadRepo.appendItem();
    const QString host1 = "ftp://random.host.com/";
    remote1->setHost(host1);
    remote1->setUsername("john");
    remote1->setPassword("doe");
    remote1->setIsSelected(true);

    auto remote2 = uploadRepo.appendItem();
    const QString host2 = "ftp://another.host.com/";
    remote2->setHost(host2);
    remote2->setUsername("doe");
    remote2->setPassword("john");
    remote2->setIsSelected(true);

    Models::ArtworksUploader &uploader = m_TestsApp.getArtworksUploader();
    uploader.uploadArtworks();

    sleepWaitUntil(10, [&uploader]() {
        return uploader.getInProgress() == false;
    });

    VERIFY(uploader.getInProgress() == false, "Uploader is still in progress");
    QCoreApplication::processEvents();

    Connectivity::UploadWatcher &watcher = uploader.accessWatcher();
    auto &failedUploads = watcher.getFailedUploads();
    qDebug() << "Failed images count is" << watcher.getFailedImagesCount();

    VERIFY(watcher.getFailedImagesCount() == 2, "Wrong failed images count");
    VERIFY(failedUploads.size() == 2, "Both hosts did not fail");

    for (auto &pair: failedUploads) {
        auto failedFiles = pair.second;
        VERIFY(failedFiles.size() == 1, "Fail Info contains more than 1 file");
        VERIFY(failedFiles[0] == filepath, "Fail Info contains wrong file");
        VERIFY((pair.first == host1) || (pair.first == host2), "Wrong host in Failed Info");
    }

    return 0;
}
