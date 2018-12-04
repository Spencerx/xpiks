#include "findandreplacemodeltest.h"

#include <memory>

#include <QLatin1String>
#include <QList>
#include <QStringList>
#include <QUrl>

#include "Artworks/artworkmetadata.h"
#include "Models/Editing/findandreplacemodel.h"
#include "QMLExtensions/uicommandid.h"

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

    const QString description = "wall inside the Wall is not a wall";

    m_TestsApp.getArtwork(0)->setDescription(description);
    m_TestsApp.getArtwork(1)->setDescription(description);
    m_TestsApp.getArtwork(1)->appendKeyword("wall e");
    // find and replace should work with both selected and not selected
    m_TestsApp.getArtwork(0)->setIsSelected(true);

    Models::FindAndReplaceModel &findAndReplaceModel = m_TestsApp.getFindAndReplaceModel();

    findAndReplaceModel.setReplaceFrom("wall");
    findAndReplaceModel.setReplaceTo("wallpaper");
    findAndReplaceModel.setSearchWholeWords(true);

    m_TestsApp.dispatch(QMLExtensions::UICommandID::FindReplaceCandidates);
    VERIFY(findAndReplaceModel.getArtworksCount() == 2, "Items are missing!");

    // find and replace should not replace in not selected items
    findAndReplaceModel.setItemSelected(1, false);

    int keywordsCount = m_TestsApp.getArtwork(0)->getKeywords().size();

    findAndReplaceModel.getActionCommand(true)->execute();

    VERIFY(m_TestsApp.getArtwork(0)->getKeywords().size() == (keywordsCount - 1), "Keyword duplicate wasn't removed");
    VERIFY(m_TestsApp.getArtwork(0)->getKeywords().last() == "wallpaper", "Keyword wasn't replaced");
    VERIFY(m_TestsApp.getArtwork(0)->getDescription() == "wallpaper inside the Wall is not a wallpaper", "Description wasn't replaced");

    VERIFY(m_TestsApp.getArtwork(1)->getDescription() == description, "Description was replaced in not selected item");
    VERIFY(m_TestsApp.getArtwork(1)->getKeywords().last() == "wall e", "Keyword was replaced in not selected item");

    return 0;
}
