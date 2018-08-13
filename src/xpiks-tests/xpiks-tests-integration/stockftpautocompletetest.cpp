#include "stockftpautocompletetest.h"
#include <QUrl>
#include <QList>
#include "signalwaiter.h"
#include "xpikstestsapp.h"
#include "testshelpers.h"

QString StockFtpAutoCompleteTest::testName() {
    return QLatin1String("StockFtpAutoCompleteTest");
}

void StockFtpAutoCompleteTest::setup() {
    // ...
}

int StockFtpAutoCompleteTest::doTest() {
    auto &uploadInfos = m_TestsApp.getUploadInfoRepository();
    AutoComplete::StringsAutoCompleteModel *acModel = uploadInfos.getStocksCompletionSource();

    VERIFY(acModel->getCount() > 1, "Stocks AC model is not full");
    // TODO: uncomment after smarter ftp list rollout
    //VERIFY(uploadInfos->accessStocksList().findFtpOptions("Fotolia")->m_ZipVector == true,
    //       "Fotolia item does not contain zip attribute");

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
