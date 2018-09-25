#include "csvexporttest.h"
#include <QUrl>
#include <QDir>
#include <QList>
#include <deque>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "testshelpers.h"
#include <Helpers/filehelpers.h>
#include <csv.h>
#include "xpikstestsapp.h"

QString CsvExportTest::testName() {
    return QLatin1String("CsvExportTest");
}

void CsvExportTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int parsePlan1Csv(const QString &filepath, Models::ArtworksListModel &artworksListModel) {
#define PLAN1_COLUMNS_COUNT 4
#define COLUMNIZE1(arr) arr[0], arr[1], arr[2], arr[3]

    io::CSVReader<PLAN1_COLUMNS_COUNT,
            io::trim_chars<' ', '\t'>,
            io::double_quote_escape<',', '\"'>,
            io::ignore_overflow,
            io::empty_line_comment> csvReader(filepath.toStdString());

    std::string columns[PLAN1_COLUMNS_COUNT];

    bool success = csvReader.read_row(COLUMNIZE1(columns));
    VERIFY(success, "First line cannot be read");

    VERIFY(columns[0] == "Filename", "Column name 1 mismatch");
    VERIFY(columns[1] == "Keywords", "Column name 2 mismatch");
    VERIFY(columns[2] == "Empty", "Column name 3 mismatch");
    VERIFY(columns[3] == "Title", "Column name 4 mismatch");

    const size_t size = artworksListModel.getArtworksSize();

    for (size_t i = 0; i < size; i++) {
        std::shared_ptr<Artworks::ArtworkMetadata> artwork;
        if (!artworksListModel.tryGetArtwork(i, artwork)) { continue; }

        success = csvReader.read_row(COLUMNIZE1(columns));
        if (!success) { qWarning() << "Row cannot be read:" << i; }
        VERIFY(success, "Failed to read a row");

        VERIFY(QString::fromStdString(columns[0]) == artwork->getBaseFilename(), "Filename value does not match");
        VERIFY(QString::fromStdString(columns[1]) == artwork->getKeywordsString(), "Keywords value does not match");
        VERIFY(columns[2].empty(), "Empty column is not empty");
        VERIFY(QString::fromStdString(columns[3]) == artwork->getTitle(), "Title value does not match");
    }

    qInfo() << "Checked first file";

    return 0;
#undef PLAN1_COLUMNS_COUNT
#undef COLUMNIZE1
}

int parsePlan2Csv(const QString &filepath, Models::ArtworksListModel &artworksListModel) {
#define PLAN2_COLUMNS_COUNT 3
#define COLUMNIZE2(arr) arr[0], arr[1], arr[2]

    io::CSVReader<PLAN2_COLUMNS_COUNT,
            io::trim_chars<' ', '\t'>,
            io::double_quote_escape<',', '\"'>,
            io::ignore_overflow,
            io::empty_line_comment> csvReader(filepath.toStdString());

    std::string columns[PLAN2_COLUMNS_COUNT];

    bool success = csvReader.read_row(COLUMNIZE2(columns));
    VERIFY(success, "First line cannot be read");

    VERIFY(columns[0] == "Filename", "Column name 1 mismatch");
    VERIFY(columns[1] == "Description", "Column name 2 mismatch");
    VERIFY(columns[2] == "Keywords", "Column name 3 mismatch");

    const size_t size = artworksListModel.getArtworksSize();

    for (size_t i = 0; i < size; i++) {
        std::shared_ptr<Artworks::ArtworkMetadata> artwork;
        if (!artworksListModel.tryGetArtwork(i, artwork)) { continue; }

        success = csvReader.read_row(COLUMNIZE2(columns));
        if (!success) { qWarning() << "Row cannot be read:" << i; }
        VERIFY(success, "Failed to read a row");

        VERIFY(QString::fromStdString(columns[0]) == artwork->getBaseFilename(), "Filename value does not match");
        VERIFY(QString::fromStdString(columns[1]) == artwork->getDescription(), "Description value does not match");
        VERIFY(QString::fromStdString(columns[2]) == artwork->getKeywordsString(), "Keywords value does not match");
    }

    qInfo() << "Checked second file";

    return 0;
#undef PLAN2_COLUMNS_COUNT
#undef COLUMNIZE2
}

void setupExportPlans(std::vector<std::shared_ptr<MetadataIO::CsvExportPlan> > &exportPlans) {
    for (auto &plan: exportPlans) { Q_ASSERT(!plan->m_IsSelected); }

    std::shared_ptr<MetadataIO::CsvExportPlan> plan1(new MetadataIO::CsvExportPlan("plan1"));
    plan1->m_IsSelected = true;
    plan1->m_PropertiesToExport.emplace_back(MetadataIO::Filename);
    plan1->m_PropertiesToExport.emplace_back(MetadataIO::Keywords);
    plan1->m_PropertiesToExport.emplace_back(MetadataIO::Empty);
    plan1->m_PropertiesToExport.emplace_back(MetadataIO::Title);

    std::shared_ptr<MetadataIO::CsvExportPlan> plan2(new MetadataIO::CsvExportPlan("plan2"));
    plan2->m_IsSelected = true;
    plan2->m_PropertiesToExport.emplace_back(MetadataIO::Filename);
    plan2->m_PropertiesToExport.emplace_back(MetadataIO::Description);
    plan2->m_PropertiesToExport.emplace_back(MetadataIO::Keywords);

    exportPlans.push_back(plan1);
    exportPlans.push_back(plan2);
}

int CsvExportTest::doTest() {
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

    setupExportPlans(csvExportModel.accessExportPlans());

    csvExportModel.startExport();

    sleepWaitUntil(5, [&csvExportModel]() {
        return csvExportModel.getIsExporting() == false;
    });

    VERIFY(!csvExportModel.getIsExporting(), "CSV export is still in progress");

    QDir outputDir(directoryPath);
    const QString filename1 = "plan1-now-xpks.csv", filename2 = "plan2-now-xpks.csv";
    VERIFY(QFileInfo(outputDir.filePath(filename1)).exists(), "Export results of plan 1 do not exist");
    VERIFY(QFileInfo(outputDir.filePath(filename2)).exists(), "Export results of plan 2 do not exist");

    int result = 0;

    result += parsePlan1Csv(outputDir.filePath(filename1), m_TestsApp.getArtworksListModel());
    result += parsePlan2Csv(outputDir.filePath(filename2), m_TestsApp.getArtworksListModel());

    return result;
}

