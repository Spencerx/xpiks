#include "weirdnamesreadtest.h"
#include <QUrl>
#include <QList>
#include "signalwaiter.h"
#include "xpikstestsapp.h"

#define FILES_IN_WEIRD_DIRECTORY 4

QString WeirdNamesReadTest::testName() {
    return QLatin1String("WeirdNamesReadTest");
}

void WeirdNamesReadTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int WeirdNamesReadTest::doTest() {
    QList<QUrl> dirs;
    dirs << getDirPathForTest("images-for-tests/weird/");

    VERIFY(m_TestsApp.addDirectoriesForTest(dirs), "Failed to add directories");
    VERIFY(m_TestsApp.getArtworksCount() == FILES_IN_WEIRD_DIRECTORY, "Did not read all files!");

    return 0;
}
