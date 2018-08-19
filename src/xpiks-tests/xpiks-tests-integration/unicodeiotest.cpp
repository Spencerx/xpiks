#include "unicodeiotest.h"
#include <QUrl>
#include <QList>
#include "signalwaiter.h"
#include "xpikstestsapp.h"
#include "exiv2iohelpers.h"

QString UnicodeIoTest::testName() {
    return QLatin1String("UnicodeIoTest");
}

void UnicodeIoTest::setup() {
}

int UnicodeIoTest::doTest() {
    // reading part ---------------------------------
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/utf8-meta/Eiffel_Wikimedia_Commons.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto artwork = m_TestsApp.getArtwork(0);

    BasicMetadata basicMetadata;
    ::readMetadata(artwork->getFilepath(), basicMetadata);

    VERIFY(basicMetadata.m_Description == artwork->getDescription(), "Description does not match for reading");
    VERIFY(basicMetadata.m_Title == artwork->getTitle(), "Title does not match for reading")
    VERIFY(basicMetadata.m_Keywords == artwork->getKeywords(), "Keywords do not match for reading");

    // writing part ---------------------------------

    files.clear();
    files << setupFilePathForTest("images-for-tests/utf8-meta/Maurice_koechlin_pylone.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    artwork = m_TestsApp.getArtwork(1);

    const QString titleu8 = QStringLiteral("πύργος του Άιφελ");
    const QString description8u = QStringLiteral("První plány stavby byly zahájeny už v roce 1878.");
    QString keywordsStr = QStringLiteral("buokšts, sėmbuolu, Parīžiuo, aukštliausės, bodīnks, metās, Е́йфелева, ве́жа, 埃菲尔铁塔");
    const QStringList keywordsu8 = keywordsStr.split(", ");

    artwork->setTitle(titleu8);
    artwork->setDescription(description8u);
    artwork->setKeywords(keywordsu8);
    artwork->setIsSelected(true);

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForExport(waiter);
    QVariantMap params{{"overwrite", QVariant(true)}, {"backup", QVariant(false)}};
    m_TestsApp.dispatch(QMLExtensions::UICommandID::SaveSelected, QVariant::fromValue(params));

    VERIFY(waiter.wait(20), "Timeout exceeded for writing metadata.");
    VERIFY(m_TestsApp.checkExportSucceeded(), "Failed to export artworks");

    BasicMetadata exifMetadata, iptcMetadata, xmpMetadata;
    ::readMetadataEx(artwork->getFilepath(), xmpMetadata, exifMetadata, iptcMetadata);

    VERIFY(exifMetadata.m_Description == description8u, "Exif description does not match");

    VERIFY(iptcMetadata.m_Title == titleu8, "IPTC title does not match")
    VERIFY(iptcMetadata.m_Description == description8u, "IPTC description does not match");
    VERIFY(iptcMetadata.m_Keywords == keywordsu8, "IPTC keyowrds do not match");

    VERIFY(xmpMetadata.m_Title == titleu8, "XMP title does not match");
    VERIFY(xmpMetadata.m_Description == description8u, "XMP description does not match");
    VERIFY(xmpMetadata.m_Keywords == keywordsu8, "XMP keywords do not match");

    return 0;
}

