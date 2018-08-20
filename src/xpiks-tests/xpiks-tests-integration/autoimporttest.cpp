#include "autoimporttest.h"
#include <QUrl>
#include <QList>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString AutoImportTest::testName() {
    return QLatin1String("AutoImportTest");
}

void AutoImportTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
    m_TestsApp.getSettingsModel().setUseAutoImport(true);
}

int AutoImportTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/read-only/026.jpg");

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForImport(waiter);
    int addedCount = m_TestsApp.addFiles(files);
    VERIFY(addedCount == 1, "Failed to add files");
    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata on first import");
    VERIFY(m_TestsApp.checkImportSucceeded(), "Failed to auto import");

    QStringList expectedKeywords = QString("abstract,art,black,blue,creative,dark,decor,decoration,decorative,design,dot,drops,elegance,element,geometric,interior,light,modern,old,ornate,paper,pattern,purple,retro,seamless,style,textile,texture,vector,wall,wallpaper").split(',');

    VERIFY(expectedKeywords == m_TestsApp.getArtwork(0)->getKeywords(), "Keywords are not the same after first import!");

    return 0;
}

