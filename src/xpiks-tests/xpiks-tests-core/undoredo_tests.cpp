#include "undoredo_tests.h"
#include <QStringList>
#include <QSignalSpy>
#include "Mocks/coretestsenvironment.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/artworksrepositorymock.h"
#include "Mocks/selectedindicessourcemock.h"
#include <Commands/Files/addfilescommand.h>
#include <Commands/Files/removeselectedfilescommand.h>
#include <Commands/Editing/modifyartworkscommand.h>
#include <Commands/Editing/findandreplacetemplate.h>
#include <Models/settingsmodel.h>
#include <UndoRedo/undoredomanager.h>
#include <Common/flags.h>
#include <Artworks/artworkelement.h>
#include <Models/Editing/previewartworkelement.h>
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Models/Session/recentdirectoriesmodel.h>
#include <KeywordsPresets/presetkeywordsmodel.h>

#define SETUP_TEST \
    Mocks::CoreTestsEnvironment environment; \
    UndoRedo::UndoRedoManager undoRedoManager; \
    Mocks::CommandManagerMock commandManager(undoRedoManager); \
    Models::RecentDirectoriesModel recentDirectories(environment);\
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories);\
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository);\
    KeywordsPresets::PresetKeywordsModel keywordsPresets(environment);\
    Models::SettingsModel settingsModel;\
    Models::FilteredArtworksListModel filteredArtworksModel(\
    artworksListModel, commandManager, keywordsPresets, settingsModel);

void UndoRedoTests::undoAddCommandTest() {
    SETUP_TEST;

    QStringList filenames;
    filenames << "/path/to/test/image1.jpg";
    auto filesCollection = std::make_shared<Mocks::FilesCollectionMock>(filenames);
    auto addFilesCommand = std::make_shared<Commands::AddFilesCommand>(filesCollection,
                                                                       Common::AddFilesFlags::None,
                                                                       artworksListModel);
    commandManager.processCommand(addFilesCommand);

    QCOMPARE(addFilesCommand->getAddedCount(), filenames.length());
    QCOMPARE(filteredArtworksModel.getItemsCount(), filenames.length());

    bool undoSucceeded = undoRedoManager.undoLastAction();
    QVERIFY(undoSucceeded);

    QCOMPARE(filteredArtworksModel.getItemsCount(), 0);
}

void UndoRedoTests::undoRemoveItemsTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    artworksListModel.generateAndAddArtworks(itemsToAdd);

    Mocks::SelectedIndicesSourceMock selectedIndices({1, 2, 3});
    auto removeCommand = std::make_shared<Commands::RemoveSelectedFilesCommand>(
                             selectedIndices, artworksListModel, artworksRepository);
    commandManager.processCommand(removeCommand);

    QCOMPARE(removeCommand->getRemovedCount(), 3);
    QCOMPARE(filteredArtworksModel.getItemsCount(), itemsToAdd - 3);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    QCOMPARE(filteredArtworksModel.getItemsCount(), itemsToAdd);
}

void UndoRedoTests::undoRemoveAddFullDirectoryTest() {
    SETUP_TEST;
    settingsModel.setAutoFindVectors(false);

    int addedCount = artworksListModel.generateAndAddDirectories(1);

    artworksListModel.removeFiles(Helpers::IndicesRanges({{1, 3}}));
    QCOMPARE(filteredArtworksModel.getItemsCount(), addedCount - 3);

    artworksListModel.removeFilesFromDirectory(0);

    QCOMPARE(filteredArtworksModel.getItemsCount(), 0);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    QVERIFY(filteredArtworksModel.getItemsCount() == addedCount);
}

void UndoRedoTests::undoRemoveNotFullDirectoryTest() {
    SETUP_TEST;
    settingsModel.setAutoFindVectors(false);

    int addedCount = artworksListModel.generateAndAddDirectories(1);
    artworksRepository.unsetWasAddedAsFullDirectory(0);

    artworksListModel.removeFiles(Helpers::IndicesRanges({{1, 3}});
    QCOMPARE(filteredArtworksModel.getItemsCount(), addedCount - 3);

    artworksListModel.removeFilesFromDirectory(0);

    QCOMPARE(filteredArtworksModel.getItemsCount(), 0);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    QVERIFY(filteredArtworksModel.getItemsCount() == addedCount - 3);
}

void UndoRedoTests::undoRemoveLaterFullDirectoryTest() {
    SETUP_TEST;
    settingsModel.setAutoFindVectors(false);

    int itemsToAdd = 5;
    artworksListModel.generateAndAddArtworks(itemsToAdd, false);
    const int addedFromSecondDir = artworksRepository.getFilesCountForDirectory(1);

    artworksListModel.generateAndAddDirectories(1, itemsToAdd, false);

    const int maxCount = filteredArtworksModel.getItemsCount();
    qDebug() << "max count is" << maxCount;

    // removing selected files from 1st directory
    artworksListModel.removeFiles(Helpers::IndicesRanges({{0, 0}, {2, 2}, {4, 4}});
    QCOMPARE(filteredArtworksModel.getItemsCount(), maxCount - 3);

    artworksListModel.removeFilesFromDirectory(0);

    QCOMPARE(filteredArtworksModel.getItemsCount(), addedFromSecondDir);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    QVERIFY(filteredArtworksModel.getItemsCount() == maxCount);
}

void UndoRedoTests::undoUndoRemoveItemsTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    artworksListModel.generateAndAddArtworks(itemsToAdd);

    QVector<QPair<int, int> > indicesToRemove;
    indicesToRemove.append(qMakePair(1, 3));
    std::shared_ptr<Commands::RemoveArtworksCommand> removeArtworkCommand(new Commands::RemoveArtworksCommand(indicesToRemove, false));
    auto result = commandManagerMock.processCommand(removeArtworkCommand);
    auto removeArtworksResult = std::dynamic_pointer_cast<Commands::RemoveArtworksCommandResult>(result);
    int artworksRemovedCount = removeArtworksResult->m_RemovedArtworksCount;

    QCOMPARE(artworksRemovedCount, 3);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    QCOMPARE(filteredArtworksModel.getItemsCount(), 2);
}

void UndoRedoTests::undoModifyCommandTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    artworksListModel.generateAndAddArtworks(itemsToAdd);

    QString originalTitle = "title";
    QString originalDescription = "some description here";
    QStringList originalKeywords = QString("test1,test2,test3").split(',');
    MetadataIO::ArtworksSnapshot::Container infos;

    for (int i = 0; i < itemsToAdd; ++i) {
        artworksListModel.getMockArtwork(i)->set(originalTitle, originalDescription, originalKeywords);
        infos.emplace_back(new Artworks::ArtworkMetadataLocker(artworksListModel.getArtwork(i)));
    }

    artworksListModel.getArtwork(0)->setModified();

    auto flags = Common::CombinedEditFlags::EditEverything;
    QString otherDescription = "brand new description";
    QString otherTitle = "other title";
    QStringList otherKeywords = QString("another,keywords,here").split(',');
    std::shared_ptr<Commands::CombinedEditCommand> combinedEditCommand(
        new Commands::CombinedEditCommand(flags, infos, otherDescription, otherTitle, otherKeywords));
    auto result = commandManagerMock.processCommand(combinedEditCommand);
    auto combinedEditResult = std::dynamic_pointer_cast<Commands::CombinedEditCommandResult>(result);
    QVector<int> indices = combinedEditResult->m_IndicesToUpdate;

    QCOMPARE(indices.length(), itemsToAdd);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    for (int i = 0; i < itemsToAdd; ++i) {
        Artworks::ArtworkMetadata *metadata = artworksListModel.getArtwork(i);
        QCOMPARE(metadata->getDescription(), originalDescription);
        QCOMPARE(metadata->getTitle(), originalTitle);
        QCOMPARE(metadata->getKeywords(), originalKeywords);
        if (i > 0) {
            QVERIFY(!artworksListModel.getArtwork(i)->isModified());
        }
    }

    QVERIFY(artworksListModel.getArtwork(0)->isModified());
}

void UndoRedoTests::undoUndoModifyCommandTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    artworksListModel.generateAndAddArtworks(itemsToAdd);

    QString originalTitle = "title";
    QString originalDescription = "some description here";
    QStringList originalKeywords = QString("test1,test2,test3").split(',');
    MetadataIO::ArtworksSnapshot::Container infos;

    for (int i = 0; i < itemsToAdd; ++i) {
        artworksListModel.getMockArtwork(i)->set(originalTitle, originalDescription, originalKeywords);
        infos.emplace_back(new Artworks::ArtworkMetadataLocker(artworksListModel.getArtwork(i)));
    }

    artworksListModel.getArtwork(0)->setModified();

    auto flags = Common::CombinedEditFlags::EditEverything;
    QString otherDescription = "brand new description";
    QString otherTitle = "other title";
    QStringList otherKeywords = QString("another,keywords,here").split(',');
    std::shared_ptr<Commands::CombinedEditCommand> combinedEditCommand(
        new Commands::CombinedEditCommand(flags, infos, otherDescription, otherTitle, otherKeywords));
    auto result = commandManagerMock.processCommand(combinedEditCommand);
    auto combinedEditResult = std::dynamic_pointer_cast<Commands::CombinedEditCommandResult>(result);
    QVector<int> indices = combinedEditResult->m_IndicesToUpdate;

    QCOMPARE(indices.length(), itemsToAdd);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(!undoStatus);
}

void UndoRedoTests::undoPasteCommandTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    artworksListModel.generateAndAddArtworks(itemsToAdd);

    QString originalTitle = "title";
    QString originalDescription = "some description here";
    QStringList originalKeywords = QString("test1,test2,test3").split(',');
    MetadataIO::ArtworksSnapshot::Container infos;

    for (int i = 0; i < itemsToAdd; ++i) {
        artworksListModel.getMockArtwork(i)->set(originalTitle, originalDescription, originalKeywords);
        infos.emplace_back(new Artworks::ArtworkMetadataLocker(artworksListModel.getArtwork(i)));
    }

    QStringList keywordsToPaste = QStringList() << "keyword1" << "keyword2" << "keyword3";

    std::shared_ptr<Commands::PasteKeywordsCommand> pasteCommand(new Commands::PasteKeywordsCommand(infos, keywordsToPaste));
    auto result = commandManagerMock.processCommand(pasteCommand);
    auto pasteCommandResult = std::dynamic_pointer_cast<Commands::PasteKeywordsCommandResult>(result);
    QVector<int> indices = pasteCommandResult->m_IndicesToUpdate;

    QCOMPARE(indices.length(), itemsToAdd);

    QStringList merged = originalKeywords;
    merged += keywordsToPaste;

    for (int i = 0; i < itemsToAdd; ++i) {
        QCOMPARE(artworksListModel.getArtwork(i)->getDescription(), originalDescription);
        QCOMPARE(artworksListModel.getArtwork(i)->getTitle(), originalTitle);
        QCOMPARE(artworksListModel.getArtwork(i)->getKeywords(), merged);
        QVERIFY(artworksListModel.getArtwork(i)->isModified());
    }

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    for (int i = 0; i < itemsToAdd; ++i) {
        QCOMPARE(artworksListModel.getArtwork(i)->getDescription(), originalDescription);
        QCOMPARE(artworksListModel.getArtwork(i)->getTitle(), originalTitle);
        QCOMPARE(artworksListModel.getArtwork(i)->getKeywords(), originalKeywords);
        QVERIFY(!artworksListModel.getArtwork(i)->isModified());
    }
}

void UndoRedoTests::undoClearAllTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    artworksListModel.generateAndAddArtworks(itemsToAdd);

    QString originalTitle = "title";
    QString originalDescription = "some description here";
    QStringList originalKeywords = QString("test1,test2,test3").split(',');
    MetadataIO::ArtworksSnapshot::Container infos;

    for (int i = 0; i < itemsToAdd; ++i) {
        artworksListModel.getMockArtwork(i)->set(originalTitle, originalDescription, originalKeywords);
        infos.emplace_back(new Artworks::ArtworkMetadataLocker(artworksListModel.getArtwork(i)));
    }

    auto flags = Common::CombinedEditFlags::Clear | Common::CombinedEditFlags::EditEverything;

    std::shared_ptr<Commands::CombinedEditCommand> combinedEditCommand(new Commands::CombinedEditCommand(flags, infos, "", "", QStringList()));
    auto result = commandManagerMock.processCommand(combinedEditCommand);
    auto combinedEditCommandResult = std::dynamic_pointer_cast<Commands::CombinedEditCommandResult>(result);
    QVector<int> indices = combinedEditCommandResult->m_IndicesToUpdate;

    QCOMPARE(indices.length(), itemsToAdd);

    for (int i = 0; i < itemsToAdd; ++i) {
        auto *keywordsModel = artworksListModel.getArtwork(i)->getBasicModel();
        QVERIFY(keywordsModel->isDescriptionEmpty());
        QVERIFY(keywordsModel->isTitleEmpty());
        QVERIFY(keywordsModel->areKeywordsEmpty());
        QVERIFY(artworksListModel.getArtwork(i)->isModified());
    }

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    for (int i = 0; i < itemsToAdd; ++i) {
        QCOMPARE(artworksListModel.getArtwork(i)->getDescription(), originalDescription);
        QCOMPARE(artworksListModel.getArtwork(i)->getTitle(), originalTitle);
        QCOMPARE(artworksListModel.getArtwork(i)->getKeywords(), originalKeywords);
        QVERIFY(!artworksListModel.getArtwork(i)->isModified());
    }
}

void UndoRedoTests::undoClearKeywordsTest() {
    SETUP_TEST;
    int itemsToAdd = 2;
    artworksListModel.generateAndAddArtworks(itemsToAdd);

    QString originalTitle = "title";
    QString originalDescription = "some description here";
    QStringList originalKeywords = QString("test1,test2,test3").split(',');
    MetadataIO::ArtworksSnapshot::Container infos;

    for (int i = 0; i < itemsToAdd; ++i) {
        artworksListModel.getMockArtwork(i)->set(originalTitle, originalDescription, originalKeywords);
        infos.emplace_back(new Artworks::ArtworkMetadataLocker(artworksListModel.getArtwork(i)));
    }

    auto flags = Common::CombinedEditFlags::Clear | Common::CombinedEditFlags::EditKeywords;

    std::shared_ptr<Commands::CombinedEditCommand> combinedEditCommand(new Commands::CombinedEditCommand(flags, infos, "", "", QStringList()));
    auto result = commandManagerMock.processCommand(combinedEditCommand);
    auto combinedEditCommandResult = std::dynamic_pointer_cast<Commands::CombinedEditCommandResult>(result);
    QVector<int> indices = combinedEditCommandResult->m_IndicesToUpdate;

    QCOMPARE(indices.length(), itemsToAdd);

    for (int i = 0; i < itemsToAdd; ++i) {
        QCOMPARE(artworksListModel.getArtwork(i)->getDescription(), originalDescription);
        QCOMPARE(artworksListModel.getArtwork(i)->getTitle(), originalTitle);
        QVERIFY(artworksListModel.getArtwork(i)->getBasicModel()->areKeywordsEmpty());
        QVERIFY(artworksListModel.getArtwork(i)->isModified());
    }

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    for (int i = 0; i < itemsToAdd; ++i) {
        QCOMPARE(artworksListModel.getArtwork(i)->getDescription(), originalDescription);
        QCOMPARE(artworksListModel.getArtwork(i)->getTitle(), originalTitle);
        QCOMPARE(artworksListModel.getArtwork(i)->getKeywords(), originalKeywords);
        QVERIFY(!artworksListModel.getArtwork(i)->isModified());
    }
}

void UndoRedoTests::undoReplaceCommandTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    Models::FilteredArtItemsProxyModel filteredItemsModel;
    filteredItemsModel.setSourceModel(artItemsModel);
    commandManagerMock.InjectDependency(&filteredItemsModel);
    artworksListModel.generateAndAddArtworks(itemsToAdd);
    QString originalDescription = "ReplaceMe";
    QString originalTitle = "ReplaceMe";
    QStringList originalKeywords = {"ReplaceMe"};

    for (int i = 0; i < itemsToAdd; i++) {
        auto *metadata = artworksListModel.getMockArtwork(i);
        metadata->set(originalDescription, originalTitle, originalKeywords);
    }

    QString replaceTo = "Replaced";
    QString replaceFrom = "Replace";
    auto flags = Common::SearchFlags::CaseSensitive |Common::SearchFlags::Description |
                Common::SearchFlags::Title | Common::SearchFlags::Keywords;
    auto artWorksInfo = filteredItemsModel.getSearchablePreviewOriginalItems(replaceFrom, flags);
    std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(new Commands::FindAndReplaceCommand(artWorksInfo, replaceFrom, replaceTo, flags) );
    auto result = commandManagerMock.processCommand(replaceCommand);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    for (int i = 0; i < itemsToAdd; ++i) {
        Artworks::ArtworkMetadata *metadata = artworksListModel.getArtwork(i);
        QCOMPARE(metadata->getDescription(), originalDescription);
        QCOMPARE(metadata->getTitle(), originalTitle);
        QCOMPARE(metadata->getKeywords(), originalKeywords);
        QVERIFY(!artworksListModel.getArtwork(i)->isModified());
    }
}
