#include "xpikstestsapp.h"
#include "testshelpers.h"

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

void XpiksTestsApp::initialize() {
    XpiksApp::initialize();

#if defined(Q_OS_WIN)
    #if defined(APPVEYOR)
        settingsModel.setExifToolPath("c:/projects/xpiks-deps/windows-3rd-party-bin/exiftool.exe");
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
    m_ArtworkUploader.accessStocksList().setRemoteOverride(stocksFtpPath);
}
