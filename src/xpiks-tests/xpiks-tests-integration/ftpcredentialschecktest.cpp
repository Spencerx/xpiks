#include "ftpcredentialschecktest.h"

#include <QList>
#include <QSignalSpy>
#include <QString>
#include <QVariant>

#include <Models/Connectivity/artworksuploader.h>
#include <Models/Connectivity/uploadinfo.h>

#include "testshelpers.h"
#include "xpikstestsapp.h"

QString FtpCredentialsCheckTest::testName() {
    return QLatin1String("FtpCredentialsCheckTest");
}

void FtpCredentialsCheckTest::setup() {
}

int FtpCredentialsCheckTest::doTest() {
    const QString host = "127.0.0.1";

    Models::ArtworksUploader &uploader = m_TestsApp.getArtworksUploader();
    QSignalSpy spy(&uploader, &Models::ArtworksUploader::credentialsChecked);

    uploader.checkCredentials(host, "ftpuser", "ftppasswd", false, false);

    VERIFY(spy.wait(5000), "Credentials were not checked");
    VERIFY(spy.count() == 1, "Received several testing results");

    QList<QVariant> arguments = spy.takeFirst();
    VERIFY(arguments.count() == 2, "Wrong signal arguments received from QSignalSpy");
    VERIFY(arguments.at(0).toBool() == true, "Credentials check failed");
    VERIFY(arguments.at(1).toString().contains(host), "Credentials were checked for the wrong host");

    return 0;
}
