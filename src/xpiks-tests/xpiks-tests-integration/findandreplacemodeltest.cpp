#include "findandreplacemodeltest.h"
#include <QUrl>
#include <QFileInfo>
#include <QStringList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString FindAndReplaceModelTest::testName() {
    return QLatin1String("FindAndReplaceModelTest");
}

void FindAndReplaceModelTest::setup() {
}

int FindAndReplaceModelTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/mixed/026.jpg");
    files << setupFilePathForTest("images-for-tests/mixed/027.jpg");
    files << setupFilePathForTest("images-for-tests/mixed/0267.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    m_TestsApp.getArtwork(0)->setDescription("wall inside the Wall is not a wall");

    m_TestsApp.selectAllArtworks();
    m_TestsApp.dispatch(QMLExtensions::UICommandID::FindAndReplaceInSelected);

    Models::FindAndReplaceModel &findAndReplaceModel = m_TestsApp.getFindAndReplaceModel();

    findAndReplaceModel.setReplaceFrom("wall");
    findAndReplaceModel.setReplaceTo("wallpaper");
    findAndReplaceModel.setSearchWholeWords(true);

    VERIFY(findAndReplaceModel.getArtworksCount() == 2, "Items are missing!");
    findAndReplaceModel.setItemSelected(1, false);

    int keywordsCount = m_TestsApp.getArtwork(0)->getKeywords().size();

    findAndReplaceModel.replace();

    VERIFY(m_TestsApp.getArtwork(0)->getKeywords().size() == (keywordsCount - 1), "Keyword duplicate wasn't removed");
    VERIFY(m_TestsApp.getArtwork(0)->getKeywords().last() == "wallpaper", "Keyword wasn't replaced");
    VERIFY(m_TestsApp.getArtwork(0)->getDescription() == "wallpaper inside the Wall is not a wallpaper", "Description wasn't replaced");

    return 0;
}



