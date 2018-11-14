#include "reimporttest.h"

#include <memory>

#include <QLatin1String>
#include <QList>
#include <QStringList>
#include <QUrl>

#include "Artworks/artworkmetadata.h"
#include "Artworks/basickeywordsmodel.h"
#include "Common/types.h"
#include "Models/settingsmodel.h"
#include "QMLExtensions/uicommandid.h"

#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString ReimportTest::testName() {
    return QLatin1String("ReimportTest");
}

void ReimportTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int ReimportTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto artwork = m_TestsApp.getArtwork(0);
    const Common::ID_t id = artwork->getItemID();

    const QString originalDescription = artwork->getDescription();
    const QString originalTitle = artwork->getTitle();
    const QStringList originalKeywords = artwork->getKeywords();

    QStringList keywords; keywords << "picture" << "seagull" << "bird";
    QString title = "Brand new title";
    QString description = "Brand new description";
    artwork->setDescription(description);
    artwork->setTitle(title);
    artwork->getBasicModel().setKeywords(keywords);

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForImport(waiter);
    m_TestsApp.selectAllArtworks();
    m_TestsApp.dispatch(QMLExtensions::UICommandID::SetupReimportMetadata);
    VERIFY(m_TestsApp.continueReading(waiter), "Failed to reimport metadata");

    const QStringList &actualKeywords = artwork->getKeywords();
    const QString &actualTitle = artwork->getTitle();
    const QString &actualDescription = artwork->getDescription();

    VERIFY(id == artwork->getItemID(), "ID should match");
    VERIFY(actualKeywords == originalKeywords, "Original keywords are not the same");
    VERIFY(actualTitle == originalTitle, "Original title is not the same");
    VERIFY(actualDescription == originalDescription, "Original description is not the same");

    return 0;
}

