#include "presetstest.h"
#include <QDir>
#include <QThread>
#include "xpikstestsapp.h"

QString PresetsTest::testName() {
    return QLatin1String("PresetsTest");
}

void PresetsTest::setup() {
}

int PresetsTest::doTest() {
    auto &presetKeywordsModel = m_TestsApp.getKeywordsPresets();
    auto &presetKeywordsModelConfig = presetKeywordsModel.getKeywordsModelConfig();

    typedef KeywordsPresets::PresetData PresetData;
    std::vector<PresetData> presetDataVector;
    presetDataVector.push_back({QStringList() << QString("key1") << QString("key2"), QString("name1"), DEFAULT_GROUP_ID});
    presetDataVector.push_back({QStringList() << QString("key3") << QString("key4"), QString("name2"), DEFAULT_GROUP_ID});

    presetKeywordsModelConfig.initialize(presetDataVector);
    presetKeywordsModel.reload();

    presetKeywordsModel.removeKeywordAt(0, 0);
    presetKeywordsModel.appendKeyword(1, "key5");
    presetKeywordsModel.addItem();
    presetKeywordsModel.setName(2, QString("name3"));
    presetKeywordsModel.appendKeyword(2, "key6");
    presetKeywordsModel.saveToConfig();

    QVector<PresetData> goldPresetDataVector;
    goldPresetDataVector.push_back({QStringList() << QString("key2"), QString("name1"), DEFAULT_GROUP_ID});
    goldPresetDataVector.push_back({QStringList() << QString("key3") << QString("key4") << QString("key5"), QString("name2"), DEFAULT_GROUP_ID});
    goldPresetDataVector.push_back({QStringList() << QString("key6"), QString("name3"), DEFAULT_GROUP_ID});

    QThread::sleep(1);

    auto &presetDataNew = presetKeywordsModelConfig.getPresetData();
    const size_t size = presetDataNew.size();
    VERIFY((int)size == goldPresetDataVector.size(), "Error in verifying config data size");

    for (size_t i = 0; i < size; i++) {
        VERIFY(presetDataNew[i].m_Keywords == goldPresetDataVector[i].m_Keywords, "Error in verifying config data item keywords");
        VERIFY(presetDataNew[i].m_Name == goldPresetDataVector[i].m_Name, "Error in verifying config data item name");
    }

    return 0;
}
