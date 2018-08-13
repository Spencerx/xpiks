#include "savefilelegacytest.h"
#include <QUrl>
#include <QList>
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString SaveFileLegacyTest::testName() {
    return QLatin1String("SaveFileLegacyTest");
}

void SaveFileLegacyTest::setup() {
    m_TestsApp.getSettingsModel().setUseDirectExiftoolExport(false);
}

int SaveFileLegacyTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    Artworks::ArtworkMetadata *artwork = m_TestsApp.getArtwork(0);
    const Common::ID_t id = artwork->getItemID();
    Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork*>(artwork);

    VERIFY(image->getImageSize().width() == 1920, "Image width was read incorrectly");
    VERIFY(image->getImageSize().height() == 1272, "Image height was read incorrectly");

    const QStringList keywords = QStringList() << "DIRECT" << "picture" << "seagull" << "bird";
    const QString title = "DIRECT Brand new title";
    const QString description = "DIRECT Brand new description";
    artwork->setDescription(description);
    artwork->setTitle(title);
    artwork->getBasicModel()->setKeywords(keywords);
    artwork->setIsSelected(true);

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForExport(waiter);
    QVariantMap params{{"overwrite", QVariant(true)}, {"backup", QVariant(false)}};
    m_TestsApp.dispatch(QMLExtensions::UICommandID::SaveSelected, QVariant::fromValue(params));

    VERIFY(waiter.wait(20), "Timeout exceeded for writing metadata.");
    VERIFY(m_TestsApp.checkExportSucceeded(), "Failed to export artworks");

    m_TestsApp.deleteAllArtworks();
    VERIFY(m_TestsApp.getArtwork(0) == nullptr, "Failed to remove files");
    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    artwork = m_TestsApp.getArtwork(0);

    VERIFY(id != artwork->getItemID(), "ID should not match");
    VERIFY(artwork->getKeywords() == keywords, "Read keywords are not the same");
    VERIFY(artwork->getTitle() == title, "Real title is not the same");
    VERIFY(artwork->getDescription() == description, "Real description is not the same");

    return 0;
}

