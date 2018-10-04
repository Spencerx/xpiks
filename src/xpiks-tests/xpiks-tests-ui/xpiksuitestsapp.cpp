#include "xpiksuitestsapp.h"
#include "../xpiks-tests-integration/signalwaiter.h"
#include "../xpiks-tests-integration/testshelpers.h"
#include "../xpiks-tests-core/Mocks/filescollectionmock.h"
#include <Commands/Files/addfilescommand.h>
#include "fakeinitartworkstemplate.h"
#include <QMLExtensions/uicommandid.h>
#include "Artworks/imageartwork.h"

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

    m_SwitcherModel.setRemoteConfigOverride(findFullPathForTests("configs-for-tests/tests_switches.json"));
    QString csvExportPlansPath;
    if (!tryFindFullPathForTests("api/v1/csv_export_plans.json", csvExportPlansPath)) {
        if (!tryFindFullPathForTests("xpiks-api/api/v1/csv_export_plans.json", csvExportPlansPath)) {
            // fallback to copy-pasted and probably not enough frequently updated just for the sake of tests
            tryFindFullPathForTests("configs-for-tests/csv_export_plans.json", csvExportPlansPath);
        }
    }
    m_CsvExportModel.setRemoteConfigOverride(csvExportPlansPath);

    QString stocksFtpPath;
    if (!tryFindFullPathForTests("api/v1/stocks_ftp.json", stocksFtpPath)) {
        if (!tryFindFullPathForTests("xpiks-api/api/v1/stocks_ftp.json", stocksFtpPath)) {
            // fallback to copy-pasted and probably not enough frequently updated just for the sake of tests
            tryFindFullPathForTests("configs-for-tests/stocks_ftp.json", stocksFtpPath);
        }
    }
    m_UploadInfoRepository.accessStocksList().setRemoteOverride(stocksFtpPath);
}

void XpiksUITestsApp::waitInitialized() {
    LOG_DEBUG << "#";
    sleepWaitUntil(5, [this]() {
        return m_ServicesInitialized;
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
    int imagesCount = 10, vectorsCount = 5, directoriesCount = 2;
    auto files = std::make_shared<Mocks::FilesCollectionMock>(imagesCount, vectorsCount, directoriesCount);

    auto initTemplate = std::make_shared<FakeInitArtworksTemplate>();

    auto addFilesCommand = std::make_shared<Commands::AddFilesCommand>(files,
                                                                       Common::AddFilesFlags::None,
                                                                       m_ArtworksListModel,
                                                                       initTemplate);
    m_CommandManager.processCommand(addFilesCommand);
    bool success = addFilesCommand->getAddedCount() == imagesCount;

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

    return success;
}

void XpiksUITestsApp::setupUITests() {
    LOG_DEBUG << "#";

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
