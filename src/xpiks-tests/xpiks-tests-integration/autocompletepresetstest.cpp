#include "autocompletepresetstest.h"
#include <QUrl>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include "integrationtestbase.h"
#include "signalwaiter.h"
#include "xpikstestsapp.h"
#include "testshelpers.h"

#define FIRST_PRESET "shutterstock"
#define SECOND_PRESET "canstockphoto"

QString AutoCompletePresetsTest::testName() {
    return QLatin1String("AutoCompletePresetsTest");
}

void AutoCompletePresetsTest::setup() {
    m_TestsApp.getSettingsModel().setUsePresetsAutoComplete(true);
    m_TestsApp.getSettingsModel().setUseKeywordsAutoComplete(true);

    KeywordsPresets::PresetKeywordsModel &presetsModel = m_TestsApp.getKeywordsPresets();
    bool dummyAdded;
    KeywordsPresets::ID_t dummyID;
    presetsModel.addOrUpdatePreset(FIRST_PRESET, QStringList() << "test" << "another" << "keyword", dummyID, dummyAdded);
    presetsModel.addOrUpdatePreset(SECOND_PRESET, QStringList() << "yet" << "other" << "keywords", dummyID, dummyAdded);
}

void AutoCompletePresetsTest::teardown() {
    IntegrationTestBase::teardown();

    KeywordsPresets::PresetKeywordsModel &presetsModel = m_TestsApp.getKeywordsPresets();
    KeywordsPresets::ID_t id;
    if (presetsModel.tryFindPresetByFullName(FIRST_PRESET, true, id)) {
        presetsModel.removePresetByID(id);
    }

    if (presetsModel.tryFindPresetByFullName(SECOND_PRESET, true, id)) {
        presetsModel.removePresetByID(id);
    }
}

int AutoCompletePresetsTest::doTest() {
    AutoComplete::AutoCompleteService &acService = m_TestsApp.getAutoCompleteService();
    AutoComplete::KeywordsAutoCompleteModel &acModel = acService.getAutoCompleteModel();
    AutoComplete::KeywordsCompletionsModel &completionsModel = acModel.getInnerModel();

    VERIFY(acModel.getCount() == 0, "AC model was not empty");

    // --------------------------------------------------------------

    acService.generateCompletions("pr:stock", nullptr);

    sleepWaitUntil(5, [&]() {
        return completionsModel.getLastGeneratedCompletionsCount() > 0;
    });

    acModel.initializeCompletions();

    qInfo() << "Generated" << acModel.getCount() << "completions";
    qInfo() << "Completions:" << completionsModel.getLastGeneratedCompletions();

    VERIFY(acModel.getCount() == 2, "Autocomplete did not find all presets");
    VERIFY(completionsModel.containsWord(FIRST_PRESET), "AC model did not find first preset");
    VERIFY(completionsModel.containsWord(SECOND_PRESET), "AC model did not find second preset");

    acModel.clear();

    // --------------------------------------------------------------

    acService.generateCompletions("pr:shttrst", nullptr);

    sleepWaitUntil(5, [&]() {
        return completionsModel.getLastGeneratedCompletionsCount() > 0;
    });

    acModel.initializeCompletions();

    qInfo() << "Generated" << acModel.getCount() << "completions";
    qInfo() << "Completions:" << completionsModel.getLastGeneratedCompletions();

    VERIFY(acModel.getCount() == 1, "Autocomplete did not find all presets");
    VERIFY(completionsModel.containsWord(FIRST_PRESET), "Presets fuzzy search does not work");

    return 0;
}

