#include "csvdefaultexporttest.h"
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include <deque>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "../../xpiks-qt/Commands/commandmanager.h"
#include "../../xpiks-qt/Models/artitemsmodel.h"
#include "../../xpiks-qt/Models/filteredartitemsproxymodel.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/Models/artworkmetadata.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/Models/imageartwork.h"
#include "../../xpiks-qt/MetadataIO/csvexportmodel.h"
#include "../../xpiks-qt/Helpers/filehelpers.h"
#include "testshelpers.h"
#include "../../../vendors/csv/csv.h"

QString CsvDefaultExportTest::testName() {
    return QLatin1String("CsvDefaultExportTest");
}

void CsvDefaultExportTest::setup() {
    Models::SettingsModel *settingsModel = m_CommandManager->getSettingsModel();
    settingsModel->setAutoFindVectors(false);
}

int CsvDefaultExportTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_CommandManager->getArtItemsModel();
    QList<QUrl> files;
    files << getFilePathForTest("images-for-tests/vector/026.jpg") <<
             getFilePathForTest("images-for-tests/pixmap/seagull.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_CommandManager->getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add file");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    Models::FilteredArtItemsProxyModel *filteredModel = m_CommandManager->getFilteredArtItemsModel();
    filteredModel->selectFilteredArtworks();
    filteredModel->setSelectedForCsvExport();

    const QString directoryPath = QCoreApplication::applicationDirPath() + QDir::separator() + testName();
    Helpers::ensureDirectoryExists(directoryPath);
    MetadataIO::CsvExportModel *csvExportModel = m_CommandManager->getCsvExportModel();
    csvExportModel->setOutputDirectory(QUrl::fromLocalFile(directoryPath));

    auto &defaultPlans = csvExportModel->accessExportPlans();
    for (auto &p: defaultPlans) { p->m_IsSelected = true; }
    VERIFY(defaultPlans.size() > 0, "Default export plans are not initialized");

    csvExportModel->startExport();

    sleepWaitUntil(5, [&csvExportModel]() {
        return csvExportModel->getIsExporting() == false;
    });

    VERIFY(!csvExportModel->getIsExporting(), "CSV export is still in progress");

    QDir outputDir(directoryPath);
    QFileInfoList csvFiles = outputDir.entryInfoList(QStringList() << "*.csv");
    VERIFY(csvFiles.length() == defaultPlans.size(), "Number of csv files is different");
    for (auto &file: csvFiles) {
        qDebug() << "Checking" << file.absoluteFilePath();
        VERIFY(file.size() > 0, "CSV filesize is zero");
    }

    return 0;
}

