#include "ftpuploadtest.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

#include <Models/Connectivity/artworksuploader.h>
#include <Models/Connectivity/uploadinfo.h>

#include "testshelpers.h"
#include "xpikstestsapp.h"

QString FtpUploadTest::testName() {
    return QLatin1String("FtpUploadTest");
}

void FtpUploadTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
    m_TestsApp.getSettingsModel().setVerboseUpload(true);
}

int FtpUploadTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/img_0007.jpg")
          << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    QStringList pathsToCheck;
    pathsToCheck << "ftp-for-tests/img_0007.jpg"
                 << "ftp-for-tests/seagull.jpg";

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    Models::UploadInfoRepository &uploadRepo = m_TestsApp.getUploadInfoRepository();

    auto remote = uploadRepo.appendItem();
    remote->setHost("127.0.0.1");
    remote->setUsername("ftpuser");
    uploadRepo.setData(uploadRepo.index(0),
                       QVariant::fromValue(QString("ftppasswd")),
                       Models::UploadInfoRepository::EditPasswordRole);
    //remote->setPassword("ftppasswd");
    //remote->setImagesDir("incoming");
    remote->setIsSelected(true);

    m_TestsApp.selectAllArtworks();
    m_TestsApp.dispatch(QMLExtensions::UICommandID::SetupUpload);

    QString fullPath;
    for (auto &s: pathsToCheck) {
        VERIFY(!tryFindFullPathForTests(s, fullPath), QString("File %1 already exists...").arg(s).toStdString().c_str());
    }

    Models::ArtworksUploader &uploader = m_TestsApp.getArtworksUploader();

    VERIFY(uploader.getItemsCount() == files.size(), "Artworks didn't get to the uploader");

    uploader.uploadArtworks();

    sleepWaitUntil(20, [&uploader]() {
        return uploader.getInProgress() == false;
    });

    VERIFY(!uploader.getInProgress(), "Upload is still in progress");
    VERIFY(!uploader.getHasErrors(), "Upload finished with errors");
    VERIFY(uploader.getUIPercent() == 100, "Upload percentage is not 100");

    for (auto &path: pathsToCheck) {
        VERIFY(tryFindFullPathForTests(path, fullPath), QString("File %1 is not found").arg(path).toStdString().c_str());
    }

    return 0;
}
