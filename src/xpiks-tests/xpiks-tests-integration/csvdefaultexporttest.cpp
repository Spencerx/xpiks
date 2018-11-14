#include "csvdefaultexporttest.h"

#include <memory>
#include <vector>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QLatin1String>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QtGlobal>

#include "Helpers/filehelpers.h"
#include "MetadataIO/csvexportmodel.h"
#include "MetadataIO/csvexportproperties.h"
#include "Models/settingsmodel.h"
#include "QMLExtensions/uicommandid.h"

#include "testshelpers.h"
#include "xpikstestsapp.h"

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
    m_TestsApp.dispatch(QMLExtensions::UICommandID::SetupCSVExportForSelected);

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

