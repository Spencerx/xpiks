#include "xpiksuitestsapp.h"

#include <memory>
#include <stdexcept>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJSValue>
#include <QList>
#include <QStandardPaths>
#include <QString>
#include <QThread>

#include <vendors/csv/csv.h>

#include "Artworks/artworkssnapshot.h"
#include "Artworks/imageartwork.h"
#include "Commands/Base/icommand.h"
#include "Commands/Files/addfilescommand.h"
#include "Commands/commandmanager.h"
#include "Common/flags.h"
#include "Common/isystemenvironment.h"
#include "Common/logging.h"
#include "Common/types.h"
#include "Helpers/indicesranges.h"
#include "Helpers/filehelpers.h"
#include "Filesystem/filescollection.h"
#include "KeywordsPresets/presetkeywordsmodel.h"
#include "KeywordsPresets/presetmodel.h"
#include "MetadataIO/csvexportmodel.h"
#include "MetadataIO/metadataioservice.h"
#include "MetadataIO/originalmetadata.h"
#include "Microstocks/stocksftplistmodel.h"
#include "Models/Artworks/artworkslistmodel.h"
#include "Models/Artworks/filteredartworkslistmodel.h"
#include "Models/Connectivity/uploadinforepository.h"
#include "Models/Editing/artworkproxymodel.h"
#include "Models/Editing/combinedartworksmodel.h"
#include "Models/Editing/currenteditablemodel.h"
#include "Models/Editing/deletekeywordsviewmodel.h"
#include "Models/Editing/quickbuffer.h"
#include "Models/settingsmodel.h"
#include "Models/switchermodel.h"
#include "QMLExtensions/uicommanddispatcher.h"
#include "QMLExtensions/uicommandid.h"
#include "Services/Maintenance/maintenanceservice.h"
#include "Services/SpellCheck/userdicteditmodel.h"
#include "Suggestion/keywordssuggestor.h"

#include "../xpiks-tests-core/Mocks/filescollectionmock.h"
#include "../xpiks-tests-integration/testshelpers.h"
#include "../xpiks-tests-integration/integrationtestsenvironment.h"

#include "fakeinitartworkstemplate.h"

namespace Artworks { class ArtworkMetadata; }
namespace Common { class ISystemEnvironment; }

std::shared_ptr<Artworks::ArtworkMetadata> createArtwork(Common::ID_t id,
                                                         const QString &title,
                                                         const QString &description,
                                                         const QStringList &keywords) {
    auto artwork = std::make_shared<Artworks::ImageArtwork>(
                       QString("/some/random/filepath/to/image_%1.jpg").arg(id.get()),
                       id,
                       0);

    MetadataIO::OriginalMetadata om;
    om.m_FilePath = artwork->getFilepath();
    om.m_Title = title;
    om.m_Description = description;
    om.m_Keywords = keywords;
    artwork->initFromOrigin(om);
    return artwork;
}

XpiksUITestsApp::XpiksUITestsApp(Common::ISystemEnvironment &environment, const QStringList &importPaths):
    XpiksApp(environment),
    m_QmlImportPaths(importPaths)
{
}

void XpiksUITestsApp::initialize() {
    XpiksApp::initialize();

    m_SettingsModel.setExifToolPath("");

    m_SwitcherModel.setRemoteConfigOverride(
                Helpers::readAllFile(
                    findFullPathForTests("configs-for-tests/tests_switches.json")));
    QString csvExportPlansPath;
    if (!tryFindFullPathForTests("api/v1/csv_export_plans.json", csvExportPlansPath)) {
        if (!tryFindFullPathForTests("xpiks-api/api/v1/csv_export_plans.json", csvExportPlansPath)) {
            // fallback to copy-pasted and probably not enough frequently updated just for the sake of tests
            tryFindFullPathForTests("configs-for-tests/csv_export_plans.json", csvExportPlansPath);
        }
    }
    m_CsvExportModel.setRemoteConfigOverride(Helpers::readAllFile(csvExportPlansPath));

    QString stocksFtpPath;
    if (!tryFindFullPathForTests("api/v1/stocks_ftp.json", stocksFtpPath)) {
        if (!tryFindFullPathForTests("xpiks-api/api/v1/stocks_ftp.json", stocksFtpPath)) {
            // fallback to copy-pasted and probably not enough frequently updated just for the sake of tests
            tryFindFullPathForTests("configs-for-tests/stocks_ftp.json", stocksFtpPath);
        }
    }
    m_UploadInfoRepository.accessStocksList().setRemoteOverride(Helpers::readAllFile(stocksFtpPath));
}

void XpiksUITestsApp::waitInitialized() {
    LOG_DEBUG << "#";
    sleepWaitUntil(5, [this]() {
        return m_ServicesInitialized.load();
    });

    // now let the main thread to fill maintenance queue
    QCoreApplication::processEvents();
    QThread::sleep(1);

    sleepWaitUntil(5, [this]() {
        return !this->m_MaintenanceService.hasPendingJobs();
    });
}

void XpiksUITestsApp::cleanupTest() {
    LOG_DEBUG << "#";
    const QString exiftoolPath = m_SettingsModel.getExifToolPath();
    {
        cleanupModels();
    }
    m_SettingsModel.setExifToolPath(exiftoolPath);
}

void XpiksUITestsApp::cleanup() {
    LOG_DEBUG << "#";
    m_ArtworksListModel
            .removeMetadata(Helpers::IndicesRanges(m_ArtworksListModel.getArtworksSize()),
                            Common::ArtworkEditFlags::Clear | Common::ArtworkEditFlags::EditEverything)
            ->execute();

    m_ArtworkProxyModel.clearModel();
    m_CombinedArtworksModel.clearModel();
    m_CsvExportModel.clearModel();
    m_UploadInfoRepository.clear();
    m_DeleteKeywordsModel.clearModel();
    m_KeywordsSuggestor.clear();
    m_QuickBuffer.resetModel();
    m_UserDictEditModel.clearModel();
}

bool XpiksUITestsApp::setupCommonFiles() {
    LOG_DEBUG << "#";

    m_MetadataIOService.writeArtworks(
                Artworks::ArtworksSnapshot(
    {
                        createArtwork(Common::ID_t(0), "Unexpected title", "Unexpected description",
                        QStringList() << "graphic" << "vector" << "illustration"),
                        createArtwork(Common::ID_t(1), "Random title", "Random description",
                        QStringList() << "graphic" << "person" << "line" << "vector"),
                        createArtwork(Common::ID_t(2), "Expected title", "Expected description",
                        QStringList() << "art" << "people" << "line" << "vector")
                    }));

    return true;
}

void XpiksUITestsApp::setupUITests(bool realFiles) {
    LOG_INFO << "real files:" << realFiles;
    if (!realFiles) { addFakeFiles(); }
    else { addRealFiles(); }

    std::shared_ptr<Artworks::ArtworkMetadata> artwork;
    m_ArtworksListModel.tryGetArtwork(0, artwork);
    m_ArtworkProxyModel.setSourceArtwork(artwork, 0);

    KeywordsPresets::ID_t id;
    bool added = false;
    m_PresetsModel.addOrUpdatePreset("interface",
                                     QStringList() << "some" << "other" << "keywords",
                                     id,
                                     added);

    m_FilteredArtworksListModel.unselectFilteredArtworks();
    // select vectors
    m_FilteredArtworksListModel.selectArtworksEx(Models::FilteredArtworksListModel::SelectVectors);
    m_UICommandDispatcher.dispatch(QMLExtensions::UICommandID::SetupEditSelectedArtworks, QJSValue());
    m_UICommandDispatcher.dispatch(QMLExtensions::UICommandID::SetupCSVExportForSelected, QJSValue());
    m_UICommandDispatcher.dispatch(QMLExtensions::UICommandID::SetupCreatingArchives, QJSValue());
    m_UICommandDispatcher.dispatch(QMLExtensions::UICommandID::SetupUpload, QJSValue());
    m_UICommandDispatcher.dispatch(QMLExtensions::UICommandID::SetupDeleteKeywordsInSelected, QJSValue());

    m_UICommandDispatcher.processAll();

    m_CurrentEditableModel.clearCurrentItem();
}

QString XpiksUITestsApp::getSessionDirectory() const {
    return dynamic_cast<IntegrationTestsEnvironment&>(m_Environment)
            .getSessionRoot();
}

bool XpiksUITestsApp::uploadedFilesExist() {
    QStringList files;
    files
            << "ftp-for-tests/026.zip"
            << "ftp-for-tests/027.zip";

    bool anyError = false;
    for (auto &file: files) {
        QString fullPath;
        if (!tryFindFullPathForTests(file, fullPath) ||
                !QFileInfo(fullPath).exists()) {
            anyError = true;
            break;
        }
    }
    return !anyError;
}

bool XpiksUITestsApp::csvExportIsValid(QString const &column0, QString const &column1) {
    QDir downloadsDir(getSessionDirectory());
    downloadsDir.setNameFilters(QStringList() << "Untitled-*.csv");
    auto fileList = downloadsDir.entryInfoList();
    if (fileList.size() > 1) { return false; }

#define PLAN1_COLUMNS_COUNT 2
#define COLUMNIZE1(arr) arr[0], arr[1]

    io::CSVReader<PLAN1_COLUMNS_COUNT,
            io::trim_chars<' ', '\t'>,
            io::double_quote_escape<',', '\"'>,
            io::ignore_overflow,
            io::empty_line_comment> csvReader(fileList.first().filePath().toStdString());

    std::string columns[PLAN1_COLUMNS_COUNT];

    bool verified = false;

    do {
        if (!csvReader.read_row(COLUMNIZE1(columns))) {
            LOG_WARNING << "Failed to read headers";
            break;
        }

        if (columns[0] != column0.toStdString()) {
            LOG_WARNING << "First column is unexpected";
            break;
        }

        if (columns[1] != column1.toStdString()) {
            LOG_WARNING << "Second column is unexpected";
            break;
        }

        bool anyError = false;
        for (size_t i = 0; i < 2; i++) {
            std::shared_ptr<Artworks::ArtworkMetadata> artwork;
            if (!m_ArtworksListModel.tryGetArtwork(i, artwork)) {
                LOG_WARNING << "Failed to get artwork" << i;
                continue;
            }

            if (!csvReader.read_row(COLUMNIZE1(columns))) {
                LOG_WARNING << "Failed to read row" << (i+1);
                anyError = true;
                break;
            }

            if (QString::fromStdString(columns[0]) != artwork->getBaseFilename()) {
                LOG_WARNING << "Filename property is wrong";
                anyError = true;
                break;
            }

            if (QString::fromStdString(columns[1]) != artwork->getKeywordsString()) {
                LOG_WARNING << "Keywords property is wrong";
                anyError = true;
                break;
            }
        }

        if (anyError) { break; }
        verified = true;
    } while (false);

    return verified;
#undef COLUMNIZE1
}

void XpiksUITestsApp::addFakeFiles() {
    LOG_DEBUG << "#";
    int imagesCount = 10, vectorsCount = 5, directoriesCount = 2;
    auto files = std::make_shared<Mocks::FilesCollectionMock>(imagesCount, vectorsCount, directoriesCount);
    doAddFiles(files, imagesCount);
}

void XpiksUITestsApp::addRealFiles() {
    LOG_DEBUG << "#";
    // read-only mode!
    QList<QUrl> files;
    files
            << setupFilePathForTest("images-for-tests/vector/026.jpg")
            << setupFilePathForTest("images-for-tests/vector/027.jpg")
            << setupFilePathForTest("images-for-tests/vector/026.eps")
            << setupFilePathForTest("images-for-tests/vector/027.eps");

    auto filesCollection = std::make_shared<Filesystem::FilesCollection>(files);
    doAddFiles(filesCollection, 2);
}

void XpiksUITestsApp::doAddFiles(const std::shared_ptr<Filesystem::IFilesCollection> &files, int addCount) {
    auto initTemplate = std::make_shared<FakeInitArtworksTemplate>();

    auto addFilesCommand = std::make_shared<Commands::AddFilesCommand>(files,
                                                                       Common::AddFilesFlags::None,
                                                                       m_ArtworksListModel,
                                                                       initTemplate);
    m_CommandManager.processCommand(addFilesCommand);
    bool success = addFilesCommand->getAddedCount() == addCount;
    if (!success) {
        throw std::runtime_error("Failed to import all files");
    }
}

QUrl XpiksUITestsApp::setupFilePathForTest(const QString &prefix, bool withVector) {
    return ::setupFilePathForTest(
                dynamic_cast<IntegrationTestsEnvironment&>(m_Environment).getSessionRoot(),
                prefix,
                withVector);
}
