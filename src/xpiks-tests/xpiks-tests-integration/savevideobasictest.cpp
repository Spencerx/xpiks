#include "savevideobasictest.h"
#include <QUrl>
#include <QFileInfo>
#include <QStringList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"
#include <Artworks/videoartwork.h>

QString SaveVideoBasicTest::testName() {
    return QLatin1String("SaveVideoBasicTest");
}

void SaveVideoBasicTest::setup() {
}

int SaveVideoBasicTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("videos-for-tests/Untitled.mp4");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto artwork = m_TestsApp.getArtwork(0);
    const Common::ID_t id = artwork->getItemID();
    auto video = std::dynamic_pointer_cast<Artworks::VideoArtwork>(artwork);

    VERIFY(video->getImageSize().width() == 640, "Video width was read incorrectly");
    VERIFY(video->getImageSize().height() == 400, "Video height was read incorrectly");

    QStringList keywords; keywords << "untitled" << "first" << "video";
    const QString title = "Title for the video";
    const QString description = "Description for the video";
    bool setSuccess = false;

    setSuccess = artwork->setDescription(description);
    VERIFY(setSuccess == true, "Failed to set video description");
    setSuccess = false;

    setSuccess = artwork->setTitle(title);
    VERIFY(setSuccess == true, "Failed to set video title");
    setSuccess = false;

    artwork->setKeywords(keywords);

    artwork->setIsSelected(true);

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForExport(waiter);
    m_TestsApp.dispatch(QMLExtensions::UICommandID::SaveSelected);

    VERIFY(waiter.wait(20), "Timeout exceeded for writing metadata.");
    VERIFY(m_TestsApp.checkExportSucceeded(), "Failed to export artworks");

    // --

    m_TestsApp.deleteAllArtworks();
    VERIFY(m_TestsApp.getArtworksCount() == 0, "Failed to remove files");
    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    artwork = m_TestsApp.getArtwork(0);

    VERIFY(id != artwork->getItemID(), "ID should not match");
    VERIFY(artwork->getKeywords() == keywords, "Read keywords are not the same");
    VERIFY(artwork->getTitle() == title, "Real title is not the same");
    VERIFY(artwork->getDescription() == description, "Real description is not the same");

    return 0;
}

