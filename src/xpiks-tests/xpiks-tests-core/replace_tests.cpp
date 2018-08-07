#include "replace_tests.h"
#include <QString>
#include <QtAlgorithms>
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/coretestsenvironment.h"
#include "Mocks/artworksrepositorymock.h"
#include <Common/flags.h>
#include <Commands/Editing/findandreplacetemplate.h>
#include <Commands/Editing/modifyartworkscommand.h>
#include <Models/Artworks/artworksrepository.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Models/Editing/previewartworkelement.h>
#include <Models/Session/recentdirectoriesmodel.h>
#include <Models/settingsmodel.h>
#include <UndoRedo/undoredomanager.h>
#include <KeywordsPresets/presetkeywordsmodel.h>

#define DECLARE_MODELS_AND_GENERATE(count) \
    Mocks::CoreTestsEnvironment environment; \
    Models::RecentDirectoriesModel recentDirectories(environment);\
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories); \
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository); \
    UndoRedo::UndoRedoManager undoRedoManager;\
    Mocks::CommandManagerMock commandManager(undoRedoManager); \
    KeywordsPresets::PresetKeywordsModel keywordsPresets(environment);\
    Models::SettingsModel settingsModel(environment);\
    Models::FilteredArtworksListModel filteredItemsModel(\
    artworksListModel, commandManager, keywordsPresets, settingsModel); \
    artworksListModel.generateAndAddArtworks(count);

void ReplaceTests::replaceTrivialTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "Replace";
    QString replaceTo = "Replaced";
    QString initString = "ReplaceMe";
    QString finalString = "ReplacedMe";

    auto flags = Common::SearchFlags::CaseSensitive |
                Common::SearchFlags::Description |
                Common::SearchFlags::Title |
                Common::SearchFlags::Keywords;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        artwork->set(initString, initString, QStringList() << initString);
    }

    using namespace Commands;
    commandManager.processCommand(
                std::make_shared<ModifyArtworksCommand>(
                    std::move(artworksListModel.createArtworksSnapshot()),
                    std::make_shared<FindAndReplaceTemplate>(replaceFrom,
                                                             replaceTo,
                                                             flags)));

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        QCOMPARE(artwork->getDescription(), finalString);
        QCOMPARE(artwork->getTitle(), finalString);
        QCOMPARE(artwork->getKeywords()[0], finalString);
        QVERIFY(artwork->isModified());
    }
}

void ReplaceTests::noReplaceTrivialTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "Noreplace";
    QString replaceTo = "Replaced";
    QString initString = "ReplaceMe";
    QString finalString = "ReplaceMe";

    auto flags = Common::SearchFlags::CaseSensitive |
                Common::SearchFlags::Description |
                Common::SearchFlags::Title |
                Common::SearchFlags::Keywords;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        artwork->set(initString, initString, QStringList() << initString);
    }

    using namespace Commands;
    commandManager.processCommand(
                std::make_shared<ModifyArtworksCommand>(
                    std::move(artworksListModel.createArtworksSnapshot()),
                    std::make_shared<FindAndReplaceTemplate>(replaceFrom,
                                                             replaceTo,
                                                             flags)));

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        QCOMPARE(artwork->getDescription(), finalString);
        QCOMPARE(artwork->getTitle(), finalString);
        QCOMPARE(artwork->getKeywords()[0], finalString);
        QVERIFY(!artwork->isModified());
    }
}

void ReplaceTests::caseSensitiveTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "rePLace";
    QString replaceTo = "Replaced";
    QString initString = "ReplaceMe";
    QString finalString = "ReplacedMe";

    auto flags = Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        artwork->set(initString, initString, QStringList() << initString);
    }

    using namespace Commands;
    commandManager.processCommand(
                std::make_shared<ModifyArtworksCommand>(
                    std::move(artworksListModel.createArtworksSnapshot()),
                    std::make_shared<FindAndReplaceTemplate>(replaceFrom,
                                                             replaceTo,
                                                             flags)));

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        QCOMPARE(artwork->getDescription(), finalString);
        QCOMPARE(artwork->getTitle(), finalString);
        QCOMPARE(artwork->getKeywords()[0], finalString);
        QVERIFY(artwork->isModified());
    }
}

void ReplaceTests::replaceTitleTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "Replace";
    QString replaceTo = "Replaced";
    QString initString = "ReplaceMe";
    QString finalString = "ReplacedMe";

    auto flags = Common::SearchFlags::CaseSensitive | Common::SearchFlags::Title;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        artwork->set(initString, initString, QStringList() << initString);
    }

    using namespace Commands;
    commandManager.processCommand(
                std::make_shared<ModifyArtworksCommand>(
                    std::move(artworksListModel.createArtworksSnapshot()),
                    std::make_shared<FindAndReplaceTemplate>(replaceFrom,
                                                             replaceTo,
                                                             flags)));

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        QCOMPARE(artwork->getDescription(), initString);
        QCOMPARE(artwork->getTitle(), finalString);
        QCOMPARE(artwork->getKeywords()[0], initString);
        QVERIFY(artwork->isModified());
    }
}

void ReplaceTests::replaceKeywordsTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "keywordOld";
    QString replaceTo = "keywordNew";
    QString replaceToLower = replaceTo.toLower();

    auto flags = Common::SearchFlags::CaseSensitive |
            Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        artwork->set(QString("title"), QString("description"),
                             QStringList() << replaceToLower << "dummyKey" << replaceFrom);
    }

    using namespace Commands;
    commandManager.processCommand(
                std::make_shared<ModifyArtworksCommand>(
                    std::move(artworksListModel.createArtworksSnapshot()),
                    std::make_shared<FindAndReplaceTemplate>(replaceFrom,
                                                             replaceTo,
                                                             flags)));

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        QCOMPARE(artwork->getDescription(), QString("description"));
        QCOMPARE(artwork->getTitle(), QString("title"));

        QStringList test = artwork->getKeywords();
        QStringList gold;

        gold << replaceToLower << "dummyKey";
        qSort(gold.begin(), gold.end());
        qSort(test.begin(), test.end());

        QCOMPARE(gold, test);
        QVERIFY(artwork->isModified());
    }
}

void ReplaceTests::replaceToSpaceTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = " vector ";
    QString replaceTo = " ";

    auto flags = Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        artwork->set(QString("A vector can be found here"), QString("And here"),
                             QStringList());
    }

    using namespace Commands;
    commandManager.processCommand(
                std::make_shared<ModifyArtworksCommand>(
                    std::move(artworksListModel.createArtworksSnapshot()),
                    std::make_shared<FindAndReplaceTemplate>(replaceFrom,
                                                             replaceTo,
                                                             flags)));

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        QCOMPARE(artwork->getDescription(), QString("And here"));
        QCOMPARE(artwork->getTitle(), QString("A can be found here"));
        QVERIFY(artwork->isModified());
    }
}

void ReplaceTests::replaceToNothingTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = " vector ";
    QString replaceTo = "";

    auto flags = Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        artwork->set(QString("A vector can be found here"), QString("And here vector  as well"),
                             QStringList());
    }

    using namespace Commands;
    commandManager.processCommand(
                std::make_shared<ModifyArtworksCommand>(
                    std::move(artworksListModel.createArtworksSnapshot()),
                    std::make_shared<FindAndReplaceTemplate>(replaceFrom,
                                                             replaceTo,
                                                             flags)));

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        QCOMPARE(artwork->getDescription(), QString("And here as well"));
        QCOMPARE(artwork->getTitle(), QString("Acan be found here"));
        QVERIFY(artwork->isModified());
    }
}

void ReplaceTests::spacesReplaceCaseSensitiveTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = " vector ";
    QString replaceTo = " ";

    auto flags = Common::SearchFlags::CaseSensitive |
            Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        artwork->set(QString("A Vector can be found here"), QString("And vector here"),
                             QStringList());
    }

    using namespace Commands;
    commandManager.processCommand(
                std::make_shared<ModifyArtworksCommand>(
                    std::move(artworksListModel.createArtworksSnapshot()),
                    std::make_shared<FindAndReplaceTemplate>(replaceFrom,
                                                             replaceTo,
                                                             flags)));

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        QCOMPARE(artwork->getDescription(), QString("And here"));
        QCOMPARE(artwork->getTitle(), QString("A Vector can be found here"));
        QVERIFY(artwork->isModified());
    }
}

void ReplaceTests::spaceReplaceCaseSensitiveNoReplaceTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = " Vector ";
    QString replaceTo = " ";

    auto flags = Common::SearchFlags::CaseSensitive |
            Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        artwork->set(QString("A vector can be found here"), QString("And vector here"),
                             QStringList());
    }

    using namespace Commands;
    commandManager.processCommand(
                std::make_shared<ModifyArtworksCommand>(
                    std::move(artworksListModel.createArtworksSnapshot()),
                    std::make_shared<FindAndReplaceTemplate>(replaceFrom,
                                                             replaceTo,
                                                             flags)));

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        QCOMPARE(artwork->getDescription(), QString("And vector here"));
        QCOMPARE(artwork->getTitle(), QString("A vector can be found here"));
        QVERIFY(!artwork->isModified());
    }
}

void ReplaceTests::replaceSpacesToWordsTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "  ";
    QString replaceTo = "word";

    auto flags = Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        artwork->set(QString("A   here"), QString("And    here"),
                             QStringList());
    }

    using namespace Commands;
    commandManager.processCommand(
                std::make_shared<ModifyArtworksCommand>(
                    std::move(artworksListModel.createArtworksSnapshot()),
                    std::make_shared<FindAndReplaceTemplate>(replaceFrom,
                                                             replaceTo,
                                                             flags)));

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        QCOMPARE(artwork->getDescription(), QString("Andwordwordhere"));
        QCOMPARE(artwork->getTitle(), QString("Aword here"));
    }
}

void ReplaceTests::replaceSpacesToSpacesTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "  ";
    QString replaceTo = " ";

    auto flags = Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        artwork->set(QString("A   here"), QString("And    here"),
                             QStringList());
    }

    using namespace Commands;
    commandManager.processCommand(
                std::make_shared<ModifyArtworksCommand>(
                    std::move(artworksListModel.createArtworksSnapshot()),
                    std::make_shared<FindAndReplaceTemplate>(replaceFrom,
                                                             replaceTo,
                                                             flags)));

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        QCOMPARE(artwork->getDescription(), QString("And  here"));
        QCOMPARE(artwork->getTitle(), QString("A  here"));
    }
}

void ReplaceTests::replaceKeywordsToEmptyTest() {
    const int itemsToGenerate = 5;
    DECLARE_MODELS_AND_GENERATE(itemsToGenerate);

    QString replaceFrom = "vector";
    QString replaceTo = " ";

    auto flags = Common::SearchFlags::CaseSensitive |
            Common::SearchFlags::Description |
            Common::SearchFlags::Title |
            Common::SearchFlags::Keywords |
            Common::SearchFlags::IncludeSpaces;

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        artwork->set(QString("A Vector can be found here"), QString("And vector here"),
                             QStringList() << "a vector here" << " vector ");
    }

    using namespace Commands;
    commandManager.processCommand(
                std::make_shared<ModifyArtworksCommand>(
                    std::move(artworksListModel.createArtworksSnapshot()),
                    std::make_shared<FindAndReplaceTemplate>(replaceFrom,
                                                             replaceTo,
                                                             flags)));

    for (int i = 0; i < itemsToGenerate; i++) {
        auto *artwork = artworksListModel.getMockArtwork(i);
        QCOMPARE(artwork->getDescription(), QString("And   here"));
        QCOMPARE(artwork->getTitle(), QString("A Vector can be found here"));

        QStringList test = artwork->getKeywords();
        QStringList gold;
        gold << "a here";

        QCOMPARE(gold, test);
        QVERIFY(artwork->isModified());
    }
}

