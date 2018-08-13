#include "csvdefaultexporttest.h"
#include <QUrl>
#include <QList>
#include <QDir>
#include <QDebug>
#include <deque>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "testshelpers.h"
#include <csv.h>
#include "xpikstestsapp.h"
#include <Helpers/filehelpers.h>

QString CsvDefaultExportTest::testName() {
    return QLatin1String("CsvDefaultExportTest");
}

void CsvDefaultExportTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int CsvDefaultExportTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg") <<
             setupFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    m_TestsApp.selectAllArtworks();
    m_TestsApp.dispatch(QMLExtensions::UICommandID::ExportSelectedToCSV);

    const QString directoryPath = QCoreApplication::applicationDirPath() + QDir::separator() + testName();
    Helpers::ensureDirectoryExists(directoryPath);

    MetadataIO::CsvExportModel &csvExportModel = m_TestsApp.getCsvExportModel();
    csvExportModel.setOutputDirectory(QUrl::fromLocalFile(directoryPath));

    auto &defaultPlans = csvExportModel.accessExportPlans();
    for (auto &p: defaultPlans) { p->m_IsSelected = true; }
    VERIFY(defaultPlans.size() > 0, "Default export plans are not initialized");

    csvExportModel.startExport();

    sleepWaitUntil(5, [&csvExportModel]() {
        return csvExportModel.getIsExporting() == false;
    });

    VERIFY(!csvExportModel.getIsExporting(), "CSV export is still in progress");

    QDir outputDir(directoryPath);
    QFileInfoList csvFiles = outputDir.entryInfoList(QStringList() << "*.csv");
    VERIFY(csvFiles.length() == (int)defaultPlans.size(), "Number of csv files is different");
    for (auto &file: csvFiles) {
        qDebug() << "Checking" << file.absoluteFilePath();
        VERIFY(file.size() > 0, "CSV filesize is zero");
    }

    return 0;
}

