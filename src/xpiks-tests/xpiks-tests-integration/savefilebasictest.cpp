#include "savefilebasictest.h"
#include <QUrl>
#include <QVariant>
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString SaveFileBasicTest::testName() {
    return QLatin1String("SaveFileBasicTest");
}

void SaveFileBasicTest::setup() {
}

int SaveFileBasicTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto artwork = m_TestsApp.getArtwork(0);
    const Common::ID_t id = artwork->getItemID();
    auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);

    VERIFY(image->getImageSize().width() == 1920, "Image width was read incorrectly");
    VERIFY(image->getImageSize().height() == 1272, "Image height was read incorrectly");

    QStringList keywords; keywords << "picture" << "seagull" << "bird";
    QString title = "Brand new title";
    QString description = "Brand new description";
    artwork->setDescription(description);
    artwork->setTitle(title);
    artwork->getBasicModel().setKeywords(keywords);

    artwork->setIsSelected(true);
    SignalWaiter waiter;
    m_TestsApp.connectWaiterForExport(waiter);
    QVariantMap params{{"overwrite", QVariant(true)}, {"backup", QVariant(false)}};
    m_TestsApp.dispatch(QMLExtensions::UICommandID::SaveSelected, QVariant::fromValue(params));

    VERIFY(waiter.wait(20), "Timeout exceeded for writing metadata.");
    VERIFY(m_TestsApp.checkExportSucceeded(), "Failed to export artworks");

    m_TestsApp.deleteAllArtworks();
    VERIFY(m_TestsApp.getArtworksCount() == 0, "Failed to remove all artworks");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    artwork = m_TestsApp.getArtwork(0);
    const QStringList &actualKeywords = artwork->getKeywords();
    const QString &actualTitle = artwork->getTitle();
    const QString &actualDescription = artwork->getDescription();

    VERIFY(id != artwork->getItemID(), "ID should not match");
    VERIFY(actualKeywords == keywords, "Read keywords are not the same");
    VERIFY(actualTitle == title, "Real title is not the same");
    VERIFY(actualDescription == description, "Real description is not the same");

    return 0;
}

