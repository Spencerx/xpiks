#include "stockftpautocompletetest.h"
#include <QUrl>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "../../xpiks-qt/Commands/commandmanager.h"
#include "../../xpiks-qt/Models/artworkuploader.h"
#include "../../xpiks-qt/AutoComplete/autocompleteservice.h"
#include "../../xpiks-qt/AutoComplete/stringsautocompletemodel.h"
#include "testshelpers.h"

QString StockFtpAutoCompleteTest::testName() {
    return QLatin1String("StockFtpAutoCompleteTest");
}

void StockFtpAutoCompleteTest::setup() {
    // ...
}

int StockFtpAutoCompleteTest::doTest() {
    auto *uploadInfos = m_CommandManager->getUploadInfoRepository();
    AutoComplete::StringsAutoCompleteModel *acModel = uploadInfos->getStocksCompletionSource();

    VERIFY(acModel->getCount() > 1, "Stocks AC model is not full");

    acModel->setSearchTerm("shttrs");

    sleepWaitUntil(5, [&]() {
        return acModel->getCount() == 1;
    });

    VERIFY(acModel->getCount() == 1, "Stocks auto completion failed to filter out result");
    bool selectedFirst = acModel->moveSelectionDown();
    VERIFY(selectedFirst, "Failed to select first completion");
    acModel->acceptSelected();
    VERIFY(acModel->getCompletion(0) == "Shutterstock", "Wrong completion found");

    return 0;
}
