#include "autocompletepresetstest.h"

#include <QDebug>
#include <QLatin1String>
#include <QObject>
#include <QStringList>
#include <QtGlobal>

#include "KeywordsPresets/presetkeywordsmodel.h"
#include "KeywordsPresets/presetmodel.h"
#include "Models/settingsmodel.h"
#include "Services/AutoComplete/autocompletemodel.h"
#include "Services/AutoComplete/autocompleteservice.h"
#include "Services/AutoComplete/keywordsautocompletemodel.h"

#include "signalwaiter.h"
#include "xpikstestsapp.h"


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
    auto &acService = m_TestsApp.getAutoCompleteService();
    auto &acModel = m_TestsApp.getKeywordsAutoCompleteModel();
    auto &completionsModel = acModel.getCompletionsSource();

    VERIFY(acModel.getCount() == 0, "AC model was not empty");

    // --------------------------------------------------------------

    SignalWaiter waiter;
    QObject::connect(&acModel, &AutoComplete::KeywordsAutoCompleteModel::completionsAvailable,
                     &waiter, &SignalWaiter::finished);
    acService.generateCompletions("pr:stock");

    VERIFY(waiter.wait(5), "Failed to get completions");

    acModel.initializeCompletions();

    qInfo() << "Generated" << acModel.getCount() << "completions";
    qInfo() << "Completions:" << completionsModel.getLastGeneratedCompletions();

    VERIFY(acModel.getCount() == 2, "Autocomplete did not find all presets");
    VERIFY(completionsModel.containsWord(FIRST_PRESET), "AC model did not find first preset");
    VERIFY(completionsModel.containsWord(SECOND_PRESET), "AC model did not find second preset");

    acModel.clear();

    // --------------------------------------------------------------

    acService.generateCompletions("pr:shttrst");

    VERIFY(waiter.wait(5), "Failed to get completions");

    acModel.initializeCompletions();

    qInfo() << "Generated" << acModel.getCount() << "completions";
    qInfo() << "Completions:" << completionsModel.getLastGeneratedCompletions();

    VERIFY(acModel.getCount() == 1, "Autocomplete did not find all presets");
    VERIFY(completionsModel.containsWord(FIRST_PRESET), "Presets fuzzy search does not work");

    return 0;
}

