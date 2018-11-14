#include "warningscombinedtest.h"

#include <memory>

#include <QDebug>
#include <QLatin1String>
#include <QList>
#include <QUrl>

#include "Artworks/artworkmetadata.h"
#include "Common/flags.h"
#include "Common/logging.h"
#include "Models/settingsmodel.h"

#include "testshelpers.h"
#include "xpikstestsapp.h"

QString WarningsCombinedTest::testName() {
    return QLatin1String("WarningsCombinedTest");
}

void WarningsCombinedTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int WarningsCombinedTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/read-only/Nokota_Horses.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    auto artwork = m_TestsApp.getArtwork(0);
    VERIFY(artwork->isInitialized(), "Artwork is not initialized after import");

    sleepWaitUntil(3, [artwork]() {
        return Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords) &&
                Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty) &&
                Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty);
    });

    VERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords), "Empty keywords not detected");
    VERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty), "Empty description not detected");
    VERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty), "Empty title not detected");

    artwork->setDescription("description");
    artwork->setTitle("title");
    artwork->appendKeyword("correct");

    LOG_INFO << "Waiting for warnings...";

    sleepWaitUntil(5, [artwork]() {
        return !Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords) &&
                !Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty) &&
                !Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty);
    });

    VERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords), "Empty keywords flag not updated");
    VERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty), "Empty description flag not updated");
    VERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty), "Empty title flag not updated");

    return 0;
}
