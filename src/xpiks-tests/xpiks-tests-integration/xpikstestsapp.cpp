#include "xpikstestsapp.h"
#include <QThread>
#include <QJSValue>
#include "testshelpers.h"
#include "signalwaiter.h"
#include <Commands/Files/removefilescommand.h>

XpiksTestsApp::XpiksTestsApp(Common::ISystemEnvironment &environment):
    XpiksApp(environment)
{
}

void XpiksTestsApp::waitInitialized() {
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

void XpiksTestsApp::cleanup() {
    const QString exiftoolPath = m_SettingsModel.getExifToolPath();
    {
        doCleanup();
    }
    m_SettingsModel.setExifToolPath(exiftoolPath);
}

bool XpiksTestsApp::checkImportSucceeded(size_t importsCount) {
    bool success = false;

    do {
        if (m_MetadataIOCoordinator.getHasErrors()) {
            LOG_WARNING << "Errors in IO Coordinator while reading";
            break;
        }

        if (m_MetadataIOCoordinator.getImportIDs().size() != importsCount) {
            LOG_WARNING << "Imports count doesn't match" << importsCount;
            break;
        }

        success = true;
    } while (false);

    return success;
}

bool XpiksTestsApp::checkExportSucceeded() {
    bool success = false;

    do {
        if (m_MetadataIOCoordinator.getHasErrors()) {
            LOG_WARNING << "Errors in IO Coordinator while writing";
            break;
        }

        success = true;
    } while (false);

    return success;
}

void XpiksTestsApp::dispatch(QMLExtensions::UICommandID::CommandID id, QVariant const &value) {
    m_UICommandDispatcher.dispatchCommand(id, value);
}

bool XpiksTestsApp::addFilesForTest(const QList<QUrl> &urls) {
    bool success = false;

    SignalWaiter waiter;
    QObject::connect(m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::metadataReadingFinished,
                     &waiter, &SignalWaiter::finished);

    do {
        int addedCount = addFiles(urls);
        if (addedCount != urls.length()) {
            LOG_WARNING << "Failed to add files:" << addedCount << "added instead of" << urls.length();
            break;
        }

        m_MetadataIOCoordinator.continueReading(true);

        if (!waiter.wait(20)) {
            LOG_WARNING << "Timeout exceeded for reading metadata.";
            break;
        }

        if (m_MetadataIOCoordinator.getHasErrors()) {
            LOG_WARNING << "Errors in IO Coordinator while reading";
            break;
        }

        if (m_MetadataIOCoordinator.getImportIDs().empty()) {
            LOG_WARNING << "Import does not have any trace";
            break;
        }

        success = true;
    } while (false);

    return success;
}

void XpiksTestsApp::deleteArtworks(Helpers::IndicesRanges const &ranges) {
    LOG_DEBUG << "#";
    m_CommandManager.processCommand(
                std::make_shared<Commands::RemoveFilesCommand>(
                    ranges,
                    m_ArtworksListModel,
                    m_ArtworksRepository));

    // delete artworks
    m_ArtworksListModel.onUndoStackEmpty();
}

bool XpiksTestsApp::undoLastAction() {
    LOG_DEBUG << "#";
    return m_UndoRedoManager.undoLastAction();
}

bool XpiksTestsApp::selectSpellSuggestions(int index) {
    LOG_DEBUG << index;
    for (int i = 0; i < m_SpellSuggestionModel.rowCount(); ++i) {
        SpellCheck::SpellSuggestionsItem *suggestionsItem = m_SpellSuggestionModel.getItem(i);
        if (suggestionsItem->rowCount() == 0) {
            LOG_WARNING << "No suggestions found for index" << i;
            return false;
        }
        suggestionsItem->setReplacementIndex(index);
    }
    return true;
}

void XpiksTestsApp::selectAllArtworks() {
    m_FilteredArtworksListModel.selectFilteredArtworks();
}

void XpiksTestsApp::connectWaiterForSpellcheck(SignalWaiter &waiter) {
    QObject::connect(m_SpellCheckerService, &SpellCheck::SpellCheckService::spellCheckQueueIsEmpty,
                     &waiter, &SignalWaiter::finished);
}

void XpiksTestsApp::connectWaiterForImport(SignalWaiter &waiter) {
    QObject::connect(m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::metadataReadingFinished,
                     &waiter, &SignalWaiter::finished);
}

void XpiksTestsApp::connectWaiterForExport(SignalWaiter &waiter) {
    QObject::connect(m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::metadataWritingFinished,
                     &waiter, &SignalWaiter::finished);
}

Artworks::ArtworkMetadata *XpiksTestsApp::getArtwork(int index) {
    return m_ArtworksListModel.getArtwork(index);
}

void XpiksTestsApp::doCleanup() {
    LOG_INTEGRATION_TESTS << "#";

    m_SpellCheckerService.cancelCurrentBatch();
    m_WarningsService.cancelCurrentBatch();
    m_MaintenanceService.cleanup();
    m_ArtworksUpdateHub.clear();
    m_AutoCompleteModel.clear();

    m_CsvExportModel.clearModel();
    m_CsvExportModel.resetModel();
    m_CombinedArtworksModel.resetModel();
    m_ZipArchiver.resetModel();
    m_ArtworkUploader.resetModel();
    m_ArtworksRepository.resetEverything();
    m_ArtItemsModel.deleteAllItems();
    m_SettingsModel.resetToDefault();
    m_SpellCheckerService.clearUserDictionary();
    m_SessionManager.clearSession();
    m_MetadataIOCoordinator.clear();
}

void XpiksTestsApp::initialize() {
    XpiksApp::initialize();

#if defined(Q_OS_WIN)
    #if defined(APPVEYOR)
        m_SettingsModel.setExifToolPath("c:/projects/xpiks-deps/windows-3rd-party-bin/exiftool.exe");
    #else
        m_SettingsModel.setExifToolPath(findFullPathForTests("xpiks-qt/deps/exiftool.exe"));
    #endif
#elif defined(Q_OS_MAC)
    m_SettingsModel.setExifToolPath(findFullPathForTests("xpiks-qt/deps/exiftool/exiftool"));
#endif

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
