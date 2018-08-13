#include "translatorbasictest.h"
#include <QDebug>
#include "signalwaiter.h"
#include "xpikstestsapp.h"

QString TranslatorBasicTest::testName() {
    return QLatin1String("TranslatorBasicTest");
}

void TranslatorBasicTest::setup() {
    // BUMP
}

int TranslatorBasicTest::doTest() {
    auto &translationManager = m_TestsApp.getTranslationManager();
    QUrl pathToDict = getFilePathForTest("dicts-for-tests/eng_fin.ifo");

    bool success = translationManager.addDictionary(pathToDict);
    VERIFY(success, "Failed to add dictionary");

    translationManager.setSelectedDictionaryIndex(0);

    SignalWaiter waiter;
    QObject::connect(&translationManager, &Translation::TranslationManager::shortTranslationChanged,
                     &waiter, &SignalWaiter::finished);

    translationManager.setQuery("test");
    VERIFY(waiter.wait(), "Timeout for waiting for translation");

    VERIFY(!translationManager.getFullTranslation().simplified().isEmpty(), "Full translation is empty");
    VERIFY(!translationManager.getShortTranslation().simplified().isEmpty(), "Short translation is empty");
    qInfo() << "Translation arrived" << translationManager.getFullTranslation();

    return 0;
}


