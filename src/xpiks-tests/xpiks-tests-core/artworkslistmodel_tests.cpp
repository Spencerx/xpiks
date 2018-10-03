#include "artworkslistmodel_tests.h"
#include <QSignalSpy>
#include "Mocks/coretestsenvironment.h"
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/artworksrepositorymock.h"
#include "Mocks/artworksupdatermock.h"
#include "Mocks/spellcheckservicemock.h"
#include "Mocks/flagsprovidermock.h"
#include "Mocks/currentartworksourcemock.h"
#include <Commands/UI/currenteditablecommands.h>
#include <Commands/artworksupdatetemplate.h>
#include <Models/Artworks/artworksrepository.h>
#include <Models/Session/recentdirectoriesmodel.h>
#include <Models/Editing/quickbuffer.h>
#include <Models/Editing/combinedartworksmodel.h>
#include <Models/Editing/currenteditablemodel.h>
#include <Models/Editing/artworkproxymodel.h>
#include <Models/Editing/deletekeywordsviewmodel.h>
#include <Services/artworksupdatehub.h>
#include <Services/SpellCheck/spellchecksuggestionmodel.h>
#include <Services/SpellCheck/spellsuggestionstarget.h>
#include <UndoRedo/undoredomanager.h>
#include <KeywordsPresets/presetkeywordsmodel.h>

#define DECLARE_MODELS_AND_GENERATE(count, withVector) \
    Mocks::CoreTestsEnvironment environment; \
    Models::RecentDirectoriesModel recentDirectories(environment);\
    recentDirectories.initialize();\
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories);\
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository);\
    artworksListModel.generateAndAddArtworks(count, withVector);

#define MODIFIED_TEST_START\
    artworksListModel.setUpdatesBlocked(false);\
    QSignalSpy artItemsModifiedSpy(&artworksListModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

#define MODIFIED_TEST_END\
    QVERIFY(artItemsModifiedSpy.count() > 0);\
    QVERIFY(artItemsModifiedSpy.takeFirst().at(2).value<QVector<int> >().contains(Models::ArtworksListModel::IsModifiedRole));

// this test can fail if the signal was emitted before this .wait()
#define MODIFIED_TEST_END_WAIT(millis)\
    QVERIFY(artItemsModifiedSpy.wait(millis));\
    QVERIFY(artItemsModifiedSpy.count() > 0);\
    QVERIFY(artItemsModifiedSpy.takeFirst().at(2).value<QVector<int> >().contains(Models::ArtworksListModel::IsModifiedRole));

void ArtworksListModelTests::removeUnavailableTest() {
    const int count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);

    for (int i = 0; i < count; ++i) {
        if (i%3 == 0) {
            artworksListModel.getMockArtwork(i)->setUnavailable();
        }
    }

    artworksListModel.purgeUnavailableFiles();

    for (int i = 0; i < artworksListModel.getArtworksSize(); ++i) {
        QVERIFY(!artworksListModel.getMockArtwork(i)->isUnavailable());
    }
}

void ArtworksListModelTests::unselectAllTest() {
    const int count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);

    for (int i = 0; i < count; ++i) {
        if (i%3 == 0) {
            artworksListModel.getMockArtwork(i)->setIsSelected(true);
        }
    }

    artworksListModel.unselectAllItems();

    for (int i = 0; i < count; ++i) {
        QVERIFY(!artworksListModel.getMockArtwork(i)->isSelected());
    }
}

void ArtworksListModelTests::modificationChangesModifiedCountTest() {
    const int count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);

    const int index = 3;

    QCOMPARE(artworksListModel.getModifiedArtworksCount(), 0);
    artworksListModel.getMockArtwork(index)->setModified();
    QCOMPARE(artworksListModel.getModifiedArtworksCount(), 1);
    artworksListModel.getMockArtwork(index)->resetModified();
    QCOMPARE(artworksListModel.getModifiedArtworksCount(), 0);
}

void ArtworksListModelTests::removeArtworkDirectorySimpleTest() {
    const int count = 11;
    DECLARE_MODELS_AND_GENERATE(count, false);

    int indexToRemove = 1;

    int firstDirCount = artworksRepository.getFilesCountForDirectory(indexToRemove);

    QCOMPARE((int)artworksListModel.getArtworksSize(), count);
    artworksListModel.removeFilesFromDirectory(indexToRemove);
    QCOMPARE((int)artworksListModel.getAvailableArtworksSize(), count - firstDirCount);
}

void ArtworksListModelTests::addRemoveOneByOneFewDirsTest() {
    // https://github.com/ribtoks/xpiks/issues/467
    const int count = 2;
    DECLARE_MODELS_AND_GENERATE(count, false);

    QCOMPARE(artworksRepository.getFilesCountForDirectory(0), 1);
    QCOMPARE(artworksRepository.getFilesCountForDirectory(1), 1);
    QCOMPARE((int)artworksListModel.getAvailableArtworksSize(), count);

    artworksListModel.removeFilesFromDirectory(0);
    artworksRepository.cleanupEmptyDirectories();
    QCOMPARE(artworksRepository.rowCount(), 1);

    artworksListModel.removeFilesFromDirectory(0);
    artworksRepository.cleanupEmptyDirectories();
    QCOMPARE(artworksRepository.rowCount(), 0);

    QCOMPARE((int)artworksListModel.getAvailableArtworksSize(), 0);
}

void ArtworksListModelTests::addRemoveOneByOneOneDirTest() {
    // https://github.com/ribtoks/xpiks/issues/467
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);

    QCOMPARE(artworksRepository.getFilesCountForDirectory(0), 1);
    QCOMPARE((int)artworksListModel.getAvailableArtworksSize(), count);

    artworksListModel.removeFilesFromDirectory(0);
    artworksRepository.cleanupEmptyDirectories();
    QCOMPARE(artworksRepository.rowCount(), 0);
    QCOMPARE((int)artworksListModel.getAvailableArtworksSize(), 0);
}

void ArtworksListModelTests::setAllSavedResetsModifiedCountTest() {
    const int count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);
    QVector<int> selectedItems;

    for (int i = 0; i < count; ++i) {
        if (i%3 == 0) {
            artworksListModel.getMockArtwork(i)->setModified();
            selectedItems.append(i);
        }
    }

    QCOMPARE(artworksListModel.getModifiedArtworksCount(), selectedItems.count());

    artworksListModel.setItemsSaved(Helpers::IndicesRanges(selectedItems));
    QCOMPARE(artworksListModel.getModifiedArtworksCount(), 0);
}

void ArtworksListModelTests::plainTextEditToEmptyKeywordsTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    UndoRedo::UndoRedoManager undoManager;
    Mocks::CommandManagerMock commandManager(undoManager);
    artworksListModel.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");
    Mocks::CurrentArtworkSourceMock artworkSource(artworksListModel.getMockArtwork(0));

    QVariantMap params{{"text", QVariant::fromValue(QString())}, {"spaceIsSeparator", QVariant(false)}};
    Commands::UI::PlainTextEditCommand(artworkSource, commandManager).execute(QVariant::fromValue(params));

    QCOMPARE(artworksListModel.getMockArtwork(0)->getKeywords().length(), 0);
}

void ArtworksListModelTests::plainTextEditToOneKeywordTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    UndoRedo::UndoRedoManager undoManager;
    Mocks::CommandManagerMock commandManager(undoManager);
    artworksListModel.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");
    Mocks::CurrentArtworkSourceMock artworkSource(artworksListModel.getMockArtwork(0));

    QString keywords = "new keyword";
    QStringList result = QStringList() << keywords;

    QVariantMap params{{"text", QVariant::fromValue(QString(keywords))}, {"spaceIsSeparator", QVariant(false)}};
    Commands::UI::PlainTextEditCommand(artworkSource, commandManager).execute(QVariant::fromValue(params));

    QCOMPARE(artworksListModel.getMockArtwork(0)->getKeywords(), result);
}

void ArtworksListModelTests::plainTextEditToSeveralKeywordsTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    UndoRedo::UndoRedoManager undoManager;
    Mocks::CommandManagerMock commandManager(undoManager);
    artworksListModel.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");
    Mocks::CurrentArtworkSourceMock artworkSource(artworksListModel.getMockArtwork(0));

    QString keywords = "new keyword, another one, new";
    QStringList result = QStringList() << "new keyword" << "another one" << "new";

    QVariantMap params{{"text", QVariant::fromValue(QString(keywords))}, {"spaceIsSeparator", QVariant(false)}};
    Commands::UI::PlainTextEditCommand(artworkSource, commandManager).execute(QVariant::fromValue(params));

    QCOMPARE(artworksListModel.getMockArtwork(0)->getKeywords(), result);
}

void ArtworksListModelTests::plainTextEditToAlmostEmptyTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    UndoRedo::UndoRedoManager undoManager;
    Mocks::CommandManagerMock commandManager(undoManager);
    artworksListModel.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");
    Mocks::CurrentArtworkSourceMock artworkSource(artworksListModel.getMockArtwork(0));

    QString keywords = ",,, , , , , ,,,,   ";
    QStringList result = QStringList();

    QVariantMap params{{"text", QVariant::fromValue(QString(keywords))}, {"spaceIsSeparator", QVariant(false)}};
    Commands::UI::PlainTextEditCommand(artworkSource, commandManager).execute(QVariant::fromValue(params));

    QCOMPARE(artworksListModel.getMockArtwork(0)->getKeywords(), result);
}

void ArtworksListModelTests::plainTextEditToMixedTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    UndoRedo::UndoRedoManager undoManager;
    Mocks::CommandManagerMock commandManager(undoManager);
    artworksListModel.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");
    Mocks::CurrentArtworkSourceMock artworkSource(artworksListModel.getMockArtwork(0));

    QString keywords = ",,, , ,word here , , ,,,,   ";
    QStringList result = QStringList() << "word here";

    QVariantMap params{{"text", QVariant::fromValue(QString(keywords))}, {"spaceIsSeparator", QVariant(false)}};
    Commands::UI::PlainTextEditCommand(artworkSource, commandManager).execute(QVariant::fromValue(params));

    QCOMPARE(artworksListModel.getMockArtwork(0)->getKeywords(), result);
}

void ArtworksListModelTests::appendKeywordEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    MODIFIED_TEST_START;

    artworksListModel.appendKeyword(0, "brand new keyword")->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::removeKeywordEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    artworksListModel.appendKeyword(0, "brand new keyword");
    artworksListModel.getMockArtwork(0)->resetModified();

    MODIFIED_TEST_START;

    artworksListModel.removeKeywordAt(0, 0)->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::removeLastKeywordEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    artworksListModel.appendKeyword(0, "brand new keyword");
    artworksListModel.getMockArtwork(0)->resetModified();

    MODIFIED_TEST_START;

    artworksListModel.removeLastKeyword(0)->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::plainTextEditEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);
    UndoRedo::UndoRedoManager undoManager;
    Mocks::CommandManagerMock commandManager(undoManager);
    Mocks::CurrentArtworkSourceMock artworkSource(artworksListModel.getMockArtwork(0),
                                                  std::make_shared<Commands::ArtworksUpdateTemplate>(artworksListModel,
                                                                                                     artworksListModel.getStandardUpdateRoles()));

    QString keywords = "brand,new,keyword";
    QVariantMap params{{"text", QVariant::fromValue(QString(keywords))}, {"spaceIsSeparator", QVariant(false)}};

    MODIFIED_TEST_START;

    Commands::UI::PlainTextEditCommand(artworkSource, commandManager).execute(QVariant::fromValue(params));

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::keywordEditEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    artworksListModel.appendKeyword(0, "keyword");
    artworksListModel.getMockArtwork(0)->resetModified();

    MODIFIED_TEST_START;

    artworksListModel.editKeyword(0, 0, "other")->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::pasteKeywordsEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    MODIFIED_TEST_START;

    artworksListModel.pasteKeywords(0, QString("other,keywords,here").split(','))->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::addSuggestedEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    int index = 0;
    UndoRedo::UndoRedoManager undoRedoManager;
    Mocks::CommandManagerMock commandManager(undoRedoManager);
    Mocks::CurrentArtworkSourceMock artworkSource(artworksListModel.getMockArtwork(index),
                                                  std::make_shared<Commands::ArtworksUpdateTemplate>(artworksListModel,
                                                                                                     artworksListModel.getStandardUpdateRoles()));

    MODIFIED_TEST_START;

    Commands::UI::AppendSuggestedKeywordsCommand(artworkSource, commandManager)
            .execute(QVariant::fromValue(QString("suggested,keywords,here").split(',')));

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::fillFromQuickBufferEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);
    UndoRedo::UndoRedoManager undoRedoManager;
    Mocks::CommandManagerMock commandManager(undoRedoManager);
    Models::CurrentEditableModel currentEditableModel;
    Models::QuickBuffer quickBuffer(currentEditableModel, commandManager);
    Common::connectTarget<std::shared_ptr<Models::ICurrentEditable>>(currentEditableModel, {artworksListModel});

    quickBuffer.setDescription("description");
    quickBuffer.setTitle("title");
    quickBuffer.setKeywords(QStringList() << "keywords" << "here");

    MODIFIED_TEST_START;

    artworksListModel.setCurrentIndex(0);
    quickBuffer.copyToCurrentEditable();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::addPresetEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    KeywordsPresets::PresetKeywordsModel presetKeywordsModel(environment);
    presetKeywordsModel.addItem("preset", QString("preset,keywords").split(','));

    MODIFIED_TEST_START;

    artworksListModel.addPreset(0, 0, presetKeywordsModel)->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::proxyModelExitEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);
    UndoRedo::UndoRedoManager undoRedoManager;
    Mocks::CommandManagerMock commandManager(undoRedoManager);
    KeywordsPresets::PresetKeywordsModel presetKeywordsModel(environment);
    Services::ArtworksUpdateHub updateHub(artworksListModel);
    Models::ArtworkProxyModel proxyModel(commandManager, presetKeywordsModel, updateHub);

    proxyModel.setSourceArtwork(artworksListModel.getMockArtwork(0), 0);
    proxyModel.setDescription("other description");

    MODIFIED_TEST_START;

    proxyModel.resetModel();
    QThread::msleep(100);
    QCoreApplication::processEvents();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::combinedEditEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(5, false);

    Services::ArtworksUpdateHub updater(artworksListModel);
    KeywordsPresets::PresetKeywordsModel presetsManager(environment);
    Models::CombinedArtworksModel combinedModel(updater, presetsManager);

    combinedModel.resetModel();
    combinedModel.setArtworks(artworksListModel.createArtworksSnapshot());
    combinedModel.setDescription("Brand new description");
    combinedModel.setTitle("Brand new title");
    combinedModel.appendKeyword("brand new keyword");

    MODIFIED_TEST_START;

    auto command = combinedModel.getActionCommand(true);
    command->execute();

    MODIFIED_TEST_END_WAIT(1000);
}

void ArtworksListModelTests::combinedModelUndoEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(5, false);

    Services::ArtworksUpdateHub updater(artworksListModel);
    KeywordsPresets::PresetKeywordsModel presetsManager(environment);
    Models::CombinedArtworksModel combinedModel(updater, presetsManager);

    combinedModel.resetModel();
    combinedModel.setArtworks(artworksListModel.createArtworksSnapshot());
    combinedModel.setDescription("Brand new description");
    combinedModel.setTitle("Brand new title");
    combinedModel.appendKeyword("brand new keyword");

    auto command = combinedModel.getActionCommand(true);
    command->execute();

    MODIFIED_TEST_START;

    command->undo();

    MODIFIED_TEST_END_WAIT(1000);
}

void ArtworksListModelTests::deleteKeywordsEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(5, false);

    Services::ArtworksUpdateHub updater(artworksListModel);
    KeywordsPresets::PresetKeywordsModel presetsManager(environment);
    Models::DeleteKeywordsViewModel deleteKeywordsModel(updater, presetsManager);

    deleteKeywordsModel.setArtworks(artworksListModel.createArtworksSnapshot());
    deleteKeywordsModel.appendKeywordToDelete("keyword1");

    MODIFIED_TEST_START;

    auto command = deleteKeywordsModel.getActionCommand(true);
    command->execute();

    MODIFIED_TEST_END_WAIT(1000);
}

void ArtworksListModelTests::deleteKeywordsUndoEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(5, false);

    Services::ArtworksUpdateHub updater(artworksListModel);
    KeywordsPresets::PresetKeywordsModel presetsManager(environment);
    Models::DeleteKeywordsViewModel deleteKeywordsModel(updater, presetsManager);

    deleteKeywordsModel.setArtworks(artworksListModel.createArtworksSnapshot());
    deleteKeywordsModel.appendKeywordToDelete("keyword1");

    auto command = deleteKeywordsModel.getActionCommand(true);
    command->execute();

    MODIFIED_TEST_START;

    command->undo();

    MODIFIED_TEST_END_WAIT(1000);
}

void ArtworksListModelTests::fixSpellingEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(2, false);

    Services::ArtworksUpdateHub updater(artworksListModel);
    Mocks::FlagsProviderMock<Common::WordAnalysisFlags> flagsProvider(Common::WordAnalysisFlags::All);
    Mocks::SpellCheckServiceMock spellCheckService(environment, flagsProvider);
    SpellCheck::SpellCheckSuggestionModel suggestionModel(spellCheckService);

    artworksListModel.foreachArtwork([](int, const std::shared_ptr<Mocks::ArtworkMetadataMock> &artwork) {
        for (auto &keyword: artwork->getBasicMetadataModel().getRawKeywords()) {
            keyword.m_IsCorrect = false;
        }
    });

    auto snapshot = artworksListModel.createArtworksSnapshot();
    suggestionModel.setupModel(
                std::make_shared<SpellCheck::ArtworksSuggestionTarget>(
                    snapshot,
                    spellCheckService,
                    updater),
                Common::SpellCheckFlags::All);

    suggestionModel.selectSomething();

    MODIFIED_TEST_START;

    suggestionModel.getActionCommand(true)->execute();

    MODIFIED_TEST_END_WAIT(1000);
}
