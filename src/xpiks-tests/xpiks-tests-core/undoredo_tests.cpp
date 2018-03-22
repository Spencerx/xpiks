#include "undoredo_tests.h"
#include <QStringList>
#include <QSignalSpy>
#include "Mocks/commandmanagermock.h"
#include "Mocks/artitemsmodelmock.h"
#include "Mocks/artworksrepositorymock.h"
#include "../../xpiks-qt/Commands/addartworkscommand.h"
#include "../../xpiks-qt/Commands/removeartworkscommand.h"
#include "../../xpiks-qt/Commands/combinededitcommand.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/UndoRedo/undoredomanager.h"
#include "../../xpiks-qt/Common/flags.h"
#include "../../xpiks-qt/Models/artworkelement.h"
#include "../../xpiks-qt/Models/previewartworkelement.h"
#include "../../xpiks-qt/Commands/pastekeywordscommand.h"
#include "../../xpiks-qt/Commands/findandreplacecommand.h"
#include "../../xpiks-qt/Models/filteredartitemsproxymodel.h"
#include "Mocks/coretestsenvironment.h"

#define SETUP_TEST \
    Mocks::CoreTestsEnvironment environment; \
    Mocks::CommandManagerMock commandManagerMock; \
    Mocks::ArtItemsModelMock artItemsMock; \
    Mocks::ArtworksRepositoryMock artworksRepository; \
    commandManagerMock.InjectDependency(&artworksRepository); \
    Models::ArtItemsModel *artItemsModel = &artItemsMock; \
    commandManagerMock.InjectDependency(artItemsModel); \
    UndoRedo::UndoRedoManager undoRedoManager; \
    commandManagerMock.InjectDependency(&undoRedoManager);

void UndoRedoTests::undoAddCommandTest() {
    SETUP_TEST;

    QStringList filenames;
    filenames << "/path/to/test/image1.jpg";

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, QStringList(), 0));
    auto result = commandManagerMock.processCommand(addArtworksCommand);
    auto addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    int newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, filenames.length());
    QCOMPARE(artItemsMock.getArtworksCount(), filenames.length());

    bool undoSucceeded = undoRedoManager.undoLastAction();
    QVERIFY(undoSucceeded);

    QCOMPARE(artItemsMock.getArtworksCount(), 0);
}

void UndoRedoTests::undoUndoAddCommandTest() {
    SETUP_TEST;

    QStringList filenames;
    filenames << "/path/to/test/image1.jpg";

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, QStringList(), 0));
    auto result = commandManagerMock.processCommand(addArtworksCommand);
    auto addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    int newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, filenames.length());
    QCOMPARE(artItemsMock.getArtworksCount(), filenames.length());

    bool undoSucceeded = undoRedoManager.undoLastAction();
    QVERIFY(undoSucceeded);

    undoSucceeded = undoRedoManager.undoLastAction();
    QVERIFY(undoSucceeded);

    QCOMPARE(artItemsMock.getArtworksCount(), filenames.length());
    for (int i = 0; i < filenames.length(); ++i) {
        QCOMPARE(artItemsMock.getArtworkFilepath(i), filenames[i]);
    }
}

void UndoRedoTests::undoUndoAddWithVectorsTest() {
    SETUP_TEST;

    QStringList filenames, vectors;
    filenames << "/path/to/test/image1.jpg" << "/path/to/test/image2.jpg" << "/path/to/test/image3.jpg";
    vectors << "/path/to/test/image3.jpg" << "/path/to/test/image1.eps";

    std::shared_ptr<Commands::AddArtworksCommand> addArtworksCommand(new Commands::AddArtworksCommand(filenames, vectors, 0));
    auto result = commandManagerMock.processCommand(addArtworksCommand);
    auto addArtworksResult = std::dynamic_pointer_cast<Commands::AddArtworksCommandResult>(result);
    int newFilesCount = addArtworksResult->m_NewFilesAdded;

    QCOMPARE(newFilesCount, filenames.length());
    QCOMPARE(artItemsMock.getArtworksCount(), filenames.length());

    bool undoSucceeded = undoRedoManager.undoLastAction();
    QVERIFY(undoSucceeded);

    undoSucceeded = undoRedoManager.undoLastAction();
    QVERIFY(undoSucceeded);

    QCOMPARE(newFilesCount, filenames.length());
    QCOMPARE(artItemsMock.getArtworksCount(), filenames.length());

    Models::ImageArtwork *image1 = artItemsMock.getMockArtwork(0);
    QVERIFY(image1->hasVectorAttached());
    QCOMPARE(image1->getAttachedVectorPath(), vectors[1]);

    Models::ImageArtwork *image2 = artItemsMock.getMockArtwork(1);
    QVERIFY(!image2->hasVectorAttached());

    Models::ImageArtwork *image3 = artItemsMock.getMockArtwork(2);
    QVERIFY(image3->hasVectorAttached());
    QCOMPARE(image3->getAttachedVectorPath(), vectors[0]);
}

void UndoRedoTests::undoRemoveItemsTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    commandManagerMock.generateAndAddArtworks(itemsToAdd);

    QVector<QPair<int, int> > indicesToRemove;
    indicesToRemove.append(qMakePair(1, 3));
    std::shared_ptr<Commands::RemoveArtworksCommand> removeArtworkCommand(new Commands::RemoveArtworksCommand(indicesToRemove, false));
    auto result = commandManagerMock.processCommand(removeArtworkCommand);
    auto removeArtworksResult = std::dynamic_pointer_cast<Commands::RemoveArtworksCommandResult>(result);
    int artworksRemovedCount = removeArtworksResult->m_RemovedArtworksCount;

    QCOMPARE(artworksRemovedCount, 3);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    QCOMPARE(artItemsMock.getArtworksCount(), itemsToAdd);
}

void UndoRedoTests::undoRemoveAddFullDirectoryTest() {
    SETUP_TEST;
    Models::SettingsModel settingsModel;
    commandManagerMock.InjectDependency(&settingsModel);
    settingsModel.setAutoFindVectors(false);

    QList<QUrl> dirs;
    dirs << QUrl::fromLocalFile(DIRECTORY_PATH);
    int addedCount = artItemsModel->addLocalDirectories(dirs);

    artItemsMock.removeItemsFromRanges({{1, 3}});
    QCOMPARE(artItemsMock.getArtworksCount(), addedCount - 3);

    artItemsMock.removeArtworksDirectory(0);

    QCOMPARE(artItemsMock.getArtworksCount(), 0);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    QVERIFY(artItemsMock.getArtworksCount() == addedCount);
}

void UndoRedoTests::undoRemoveNotFullDirectoryTest() {
    SETUP_TEST;
    Models::SettingsModel settingsModel;
    commandManagerMock.InjectDependency(&settingsModel);
    settingsModel.setAutoFindVectors(false);

    QList<QUrl> dirs;
    dirs << QUrl::fromLocalFile(DIRECTORY_PATH);
    int addedCount = artItemsModel->addLocalDirectories(dirs);

    artworksRepository.unsetWasAddedAsFullDirectory(0);

    artItemsMock.removeItemsFromRanges({{1, 3}});
    QCOMPARE(artItemsMock.getArtworksCount(), addedCount - 3);

    artItemsMock.removeArtworksDirectory(0);

    QCOMPARE(artItemsMock.getArtworksCount(), 0);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    QVERIFY(artItemsMock.getArtworksCount() == addedCount - 3);
}

void UndoRedoTests::undoRemoveLaterFullDirectoryTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    commandManagerMock.generateAndAddArtworks(itemsToAdd, false);
    const int addedFromSecondDir = artworksRepository.getFilesCountForDirectory(1);

    Models::SettingsModel settingsModel;
    commandManagerMock.InjectDependency(&settingsModel);
    settingsModel.setAutoFindVectors(false);

    QList<QUrl> dirs;
    dirs << QUrl::fromLocalFile(DIRECTORY_PATH "_0");
    artItemsModel->addLocalDirectories(dirs);

    const int maxCount = artItemsModel->getArtworksCount();
    qDebug() << "max count is" << maxCount;

    // removing selected files from 1st directory
    artItemsMock.removeItemsFromRanges({{0, 0}, {2, 2}, {4, 4}});
    QCOMPARE(artItemsMock.getArtworksCount(), maxCount - 3);

    artItemsMock.removeArtworksDirectory(0);

    QCOMPARE(artItemsMock.getArtworksCount(), addedFromSecondDir);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    QVERIFY(artItemsMock.getArtworksCount() == maxCount);
}

void UndoRedoTests::undoUndoRemoveItemsTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    commandManagerMock.generateAndAddArtworks(itemsToAdd);

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

    QCOMPARE(artItemsMock.getArtworksCount(), 2);
}

void UndoRedoTests::undoModifyCommandTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    commandManagerMock.generateAndAddArtworks(itemsToAdd);

    QString originalTitle = "title";
    QString originalDescription = "some description here";
    QStringList originalKeywords = QString("test1,test2,test3").split(',');
    MetadataIO::ArtworksSnapshot::Container infos;

    for (int i = 0; i < itemsToAdd; ++i) {
        artItemsMock.getMockArtwork(i)->set(originalTitle, originalDescription, originalKeywords);
        infos.emplace_back(new Models::ArtworkMetadataLocker(artItemsMock.getArtwork(i)));
    }

    artItemsMock.getArtwork(0)->setModified();

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
        Models::ArtworkMetadata *metadata = artItemsMock.getArtwork(i);
        QCOMPARE(metadata->getDescription(), originalDescription);
        QCOMPARE(metadata->getTitle(), originalTitle);
        QCOMPARE(metadata->getKeywords(), originalKeywords);
        if (i > 0) {
            QVERIFY(!artItemsMock.getArtwork(i)->isModified());
        }
    }

    QVERIFY(artItemsMock.getArtwork(0)->isModified());
}

void UndoRedoTests::undoUndoModifyCommandTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    commandManagerMock.generateAndAddArtworks(itemsToAdd);

    QString originalTitle = "title";
    QString originalDescription = "some description here";
    QStringList originalKeywords = QString("test1,test2,test3").split(',');
    MetadataIO::ArtworksSnapshot::Container infos;

    for (int i = 0; i < itemsToAdd; ++i) {
        artItemsMock.getMockArtwork(i)->set(originalTitle, originalDescription, originalKeywords);
        infos.emplace_back(new Models::ArtworkMetadataLocker(artItemsMock.getArtwork(i)));
    }

    artItemsMock.getArtwork(0)->setModified();

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
    commandManagerMock.generateAndAddArtworks(itemsToAdd);

    QString originalTitle = "title";
    QString originalDescription = "some description here";
    QStringList originalKeywords = QString("test1,test2,test3").split(',');
    MetadataIO::ArtworksSnapshot::Container infos;

    for (int i = 0; i < itemsToAdd; ++i) {
        artItemsMock.getMockArtwork(i)->set(originalTitle, originalDescription, originalKeywords);
        infos.emplace_back(new Models::ArtworkMetadataLocker(artItemsMock.getArtwork(i)));
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
        QCOMPARE(artItemsMock.getArtwork(i)->getDescription(), originalDescription);
        QCOMPARE(artItemsMock.getArtwork(i)->getTitle(), originalTitle);
        QCOMPARE(artItemsMock.getArtwork(i)->getKeywords(), merged);
        QVERIFY(artItemsMock.getArtwork(i)->isModified());
    }

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    for (int i = 0; i < itemsToAdd; ++i) {
        QCOMPARE(artItemsMock.getArtwork(i)->getDescription(), originalDescription);
        QCOMPARE(artItemsMock.getArtwork(i)->getTitle(), originalTitle);
        QCOMPARE(artItemsMock.getArtwork(i)->getKeywords(), originalKeywords);
        QVERIFY(!artItemsMock.getArtwork(i)->isModified());
    }
}

void UndoRedoTests::undoClearAllTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    commandManagerMock.generateAndAddArtworks(itemsToAdd);

    QString originalTitle = "title";
    QString originalDescription = "some description here";
    QStringList originalKeywords = QString("test1,test2,test3").split(',');
    MetadataIO::ArtworksSnapshot::Container infos;

    for (int i = 0; i < itemsToAdd; ++i) {
        artItemsMock.getMockArtwork(i)->set(originalTitle, originalDescription, originalKeywords);
        infos.emplace_back(new Models::ArtworkMetadataLocker(artItemsMock.getArtwork(i)));
    }

    auto flags = Common::CombinedEditFlags::Clear | Common::CombinedEditFlags::EditEverything;

    std::shared_ptr<Commands::CombinedEditCommand> combinedEditCommand(new Commands::CombinedEditCommand(flags, infos, "", "", QStringList()));
    auto result = commandManagerMock.processCommand(combinedEditCommand);
    auto combinedEditCommandResult = std::dynamic_pointer_cast<Commands::CombinedEditCommandResult>(result);
    QVector<int> indices = combinedEditCommandResult->m_IndicesToUpdate;

    QCOMPARE(indices.length(), itemsToAdd);

    for (int i = 0; i < itemsToAdd; ++i) {
        auto *keywordsModel = artItemsMock.getArtwork(i)->getBasicModel();
        QVERIFY(keywordsModel->isDescriptionEmpty());
        QVERIFY(keywordsModel->isTitleEmpty());
        QVERIFY(keywordsModel->areKeywordsEmpty());
        QVERIFY(artItemsMock.getArtwork(i)->isModified());
    }

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    for (int i = 0; i < itemsToAdd; ++i) {
        QCOMPARE(artItemsMock.getArtwork(i)->getDescription(), originalDescription);
        QCOMPARE(artItemsMock.getArtwork(i)->getTitle(), originalTitle);
        QCOMPARE(artItemsMock.getArtwork(i)->getKeywords(), originalKeywords);
        QVERIFY(!artItemsMock.getArtwork(i)->isModified());
    }
}

void UndoRedoTests::undoClearKeywordsTest() {
    SETUP_TEST;
    int itemsToAdd = 2;
    commandManagerMock.generateAndAddArtworks(itemsToAdd);

    QString originalTitle = "title";
    QString originalDescription = "some description here";
    QStringList originalKeywords = QString("test1,test2,test3").split(',');
    MetadataIO::ArtworksSnapshot::Container infos;

    for (int i = 0; i < itemsToAdd; ++i) {
        artItemsMock.getMockArtwork(i)->set(originalTitle, originalDescription, originalKeywords);
        infos.emplace_back(new Models::ArtworkMetadataLocker(artItemsMock.getArtwork(i)));
    }

    auto flags = Common::CombinedEditFlags::Clear | Common::CombinedEditFlags::EditKeywords;

    std::shared_ptr<Commands::CombinedEditCommand> combinedEditCommand(new Commands::CombinedEditCommand(flags, infos, "", "", QStringList()));
    auto result = commandManagerMock.processCommand(combinedEditCommand);
    auto combinedEditCommandResult = std::dynamic_pointer_cast<Commands::CombinedEditCommandResult>(result);
    QVector<int> indices = combinedEditCommandResult->m_IndicesToUpdate;

    QCOMPARE(indices.length(), itemsToAdd);

    for (int i = 0; i < itemsToAdd; ++i) {
        QCOMPARE(artItemsMock.getArtwork(i)->getDescription(), originalDescription);
        QCOMPARE(artItemsMock.getArtwork(i)->getTitle(), originalTitle);
        QVERIFY(artItemsMock.getArtwork(i)->getBasicModel()->areKeywordsEmpty());
        QVERIFY(artItemsMock.getArtwork(i)->isModified());
    }

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    for (int i = 0; i < itemsToAdd; ++i) {
        QCOMPARE(artItemsMock.getArtwork(i)->getDescription(), originalDescription);
        QCOMPARE(artItemsMock.getArtwork(i)->getTitle(), originalTitle);
        QCOMPARE(artItemsMock.getArtwork(i)->getKeywords(), originalKeywords);
        QVERIFY(!artItemsMock.getArtwork(i)->isModified());
    }
}

void UndoRedoTests::undoReplaceCommandTest() {
    SETUP_TEST;
    int itemsToAdd = 5;
    Models::FilteredArtItemsProxyModel filteredItemsModel;
    filteredItemsModel.setSourceModel(artItemsModel);
    commandManagerMock.InjectDependency(&filteredItemsModel);
    commandManagerMock.generateAndAddArtworks(itemsToAdd);
    QString originalDescription = "ReplaceMe";
    QString originalTitle = "ReplaceMe";
    QStringList originalKeywords = {"ReplaceMe"};

    for (int i = 0; i < itemsToAdd; i++) {
        auto *metadata = artItemsMock.getMockArtwork(i);
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
        Models::ArtworkMetadata *metadata = artItemsMock.getArtwork(i);
        QCOMPARE(metadata->getDescription(), originalDescription);
        QCOMPARE(metadata->getTitle(), originalTitle);
        QCOMPARE(metadata->getKeywords(), originalKeywords);
        QVERIFY(!artItemsMock.getArtwork(i)->isModified());
    }
}
