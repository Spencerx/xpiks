#include "artworkrepository_tests.h"
#include <QSignalSpy>
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/artworksrepositorymock.h"
#include "Mocks/commandmanagermock.h"
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Models/Artworks/artworksrepository.h>
#include <KeywordsPresets/presetkeywordsmodel.h>
#include <Models/Editing/artworkproxymodel.h>
#include <Models/settingsmodel.h>
#include <Models/Session/recentdirectoriesmodel.h>
#include <UndoRedo/undoredomanager.h>
#include "Mocks/coretestsenvironment.h"
#include "Mocks/filescollectionmock.h"

#define DECLARE_BASIC_MODELS\
    Mocks::CoreTestsEnvironment environment; \
    Models::RecentDirectoriesModel recentDirectories(environment);\
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories);

#define DECLARE_MODELS_AND_GENERATE(count, withVector) \
    DECLARE_BASIC_MODELS\
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository);\
    KeywordsPresets::PresetKeywordsModel keywordsPresets(environment);\
    Models::SettingsModel settingsModel(environment);\
    UndoRedo::UndoRedoManager undoRedoManager;\
    Mocks::CommandManagerMock commandManager(undoRedoManager);\
    Models::FilteredArtworksListModel filteredArtworksModel(\
    artworksListModel, commandManager, keywordsPresets, settingsModel);\
    artworksListModel.generateAndAddArtworks(count, withVector);

#define SETUP_SELECTION_TEST(count, dirsCount) \
    DECLARE_BASIC_MODELS\
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository);\
    UndoRedo::UndoRedoManager undoRedoManager; \
    Models::SettingsModel settingsModel(environment); \
    settingsModel.setAutoFindVectors(false); \
    artworksListModel.generateAndAddArtworksEx(count, dirsCount, false); \
    QCOMPARE((int)artworksRepository.accessRepos().size(), dirsCount);

#define CHECK_ONLY_SELECTED(selectedIndex) \
    { \
    QCOMPARE(artworksRepository.accessRepos()[selectedIndex].getIsSelectedFlag(), true);\
    const size_t size = artworksRepository.accessRepos().size(); \
    for (size_t i = 0; i < size; i++) { \
        if (i == selectedIndex) { continue; } \
        auto &dir = artworksRepository.accessRepos()[i]; \
        QVERIFY2(dir.getIsSelectedFlag() == false, QString("Directory IS selected at %1").arg(i).toStdString().data()); \
    } \
    }

#define CHECK_ALL_SELECTED \
    {\
    const size_t size = artworksRepository.accessRepos().size(); \
    for (size_t i = 0; i < size; i++) { \
        auto &dir = artworksRepository.accessRepos()[i]; \
        QVERIFY2(dir.getIsSelectedFlag() == true, QString("Directory is NOT selected at %1").arg(i).toStdString().data()); \
    } \
    }

#define CHECK_SELECTED(index) \
    QCOMPARE(artworksRepository.accessRepos()[index].getIsSelectedFlag(), true);

#define CHECK_UNSELECTED(index) \
    QCOMPARE(artworksRepository.accessRepos()[index].getIsSelectedFlag(), false);

void ArtworkRepositoryTests::simpleAccountFileTest() {
    DECLARE_BASIC_MODELS;

#ifdef Q_OS_WIN
    QString filename = "C:/path/to/some/file";
    QString directory = "C:/path/to/some";
#else
    QString filename = "/path/to/some/file";
    QString directory = "/path/to/some";
#endif
    qint64 dirID = 0;
    bool status = artworksRepository.accountFile(filename, dirID);

    QCOMPARE(status, true);
    QCOMPARE(artworksRepository.rowCount(), 1);
    QCOMPARE(artworksRepository.getDirectoryPath(0), directory);
    QCOMPARE(artworksRepository.getFilesCountForDirectory(directory), 1);
}

void ArtworkRepositoryTests::accountSameFileTest() {
    DECLARE_BASIC_MODELS;

#ifdef Q_OS_WIN
    QString filename = "C:/path/to/some/file";
    QString directory = "C:/path/to/some";
#else
    QString filename = "/path/to/some/file";
    QString directory = "/path/to/some";
#endif

    qint64 dirID = 0;
    artworksRepository.accountFile(filename, dirID);
    bool status = artworksRepository.accountFile(filename, dirID);

    QCOMPARE(status, false);
    QCOMPARE(artworksRepository.rowCount(), 1);
    QCOMPARE(artworksRepository.getDirectoryPath(0), directory);
    QCOMPARE(artworksRepository.getFilesCountForDirectory(directory), 1);
}

void ArtworkRepositoryTests::addFilesFromOneDirectoryTest() {
    DECLARE_BASIC_MODELS;

#ifdef Q_OS_WIN
    QString filenameTemplate = "C:/path/to/some/file%1.jpg";
    QString directory = "C:/path/to/some";
#else
    QString filenameTemplate = "/path/to/some/file%1.jpg";
    QString directory = "/path/to/some";
    #endif

    bool anyWrong = false;
    int count = 5;
    while (count--) {
        QString filename = filenameTemplate.arg(5 - count - 1);
        qint64 dirID = 0;
        if (!artworksRepository.accountFile(filename, dirID)) {
            anyWrong = true;
        }
    }

    QCOMPARE(anyWrong, false);
    QCOMPARE(artworksRepository.rowCount(), 1);
    QCOMPARE(artworksRepository.getFilesCountForDirectory(directory), 5);
}

void ArtworkRepositoryTests::addAndRemoveSameFileTest() {
    DECLARE_BASIC_MODELS;

#ifdef Q_OS_WIN
    QString filename = "C:/path/to/some/file";
#else
    QString filename = "/path/to/some/file";
#endif

    qint64 dirID = 0;
    bool status = artworksRepository.accountFile(filename, dirID);
    QCOMPARE(status, true);

    bool removeResult = artworksRepository.removeFile(filename, dirID);
    artworksRepository.cleanupEmptyDirectories();

    QCOMPARE(artworksRepository.rowCount(), 0);
    QCOMPARE(removeResult, true);
}

void ArtworkRepositoryTests::removeNotExistingFileTest() {
    DECLARE_BASIC_MODELS;

#ifdef Q_OS_WIN
    QString filename1 = "C:/path/to/some/file1";
    QString filename2 = "C:/path/to/some/file2";
#else
    QString filename1 = "/path/to/some/file1";
    QString filename2 = "/path/to/some/file2";
#endif

    qint64 dirID = 0;
    bool status = artworksRepository.accountFile(filename1, dirID);
    QCOMPARE(status, true);
    QCOMPARE(artworksRepository.rowCount(), 1);

    bool removeResult = artworksRepository.removeFile(filename2, dirID);
    artworksRepository.cleanupEmptyDirectories();

    QCOMPARE(removeResult, false);
    QCOMPARE(artworksRepository.rowCount(), 1);
}

void ArtworkRepositoryTests::brandNewDirectoriesCountTest() {
    DECLARE_BASIC_MODELS;

#ifdef Q_OS_WIN
    QString filename1 = "C:/path/to/some/file1";
    QString filename2 = "C:/path/to/some/file2";
#else
    QString filename1 = "/path/to/some/file1";
    QString filename2 = "/path/to/some/file2";
#endif

    QStringList files;
    files << filename1 << filename2;

    int newDirsCount = artworksRepository.getNewDirectoriesCount(files);
    QCOMPARE(newDirsCount, 1);
    QCOMPARE(artworksRepository.rowCount(), 0);
    QCOMPARE(artworksRepository.rowCount(), 0);
}

void ArtworkRepositoryTests::differentNewDirectoriesCountTest() {
    DECLARE_BASIC_MODELS;

#ifdef Q_OS_WIN
    QString filename1 = "C:/path/to/some/file1";
    QString filename2 = "C:/path/to/some/other/file2";
#else
    QString filename1 = "/path/to/some/file1";
    QString filename2 = "/path/to/some/other/file2";
#endif

    QStringList files;
    files << filename1 << filename2;

    int newDirsCount = artworksRepository.getNewDirectoriesCount(files);
    QCOMPARE(newDirsCount, files.length());
    QCOMPARE(artworksRepository.rowCount(), 0);
    QCOMPARE(artworksRepository.rowCount(), 0);
}

void ArtworkRepositoryTests::newFilesCountTest() {
    DECLARE_BASIC_MODELS;

#ifdef Q_OS_WIN
    QString filename1 = "C:/path/to/some/file1";
    QString filename2 = "C:/path/to/some/file2";
#else
    QString filename1 = "/path/to/some/file1";
    QString filename2 = "/path/to/some/file2";
#endif

    QStringList files;
    files << filename1 << filename2;
    auto filesSource = std::make_shared<Mocks::FilesCollectionMock>(files);

    int newFilesCount = artworksRepository.getNewFilesCount(filesSource);
    QCOMPARE(newFilesCount, files.length());
    QCOMPARE(artworksRepository.rowCount(), 0);
    QCOMPARE(artworksRepository.rowCount(), 0);
}

void ArtworkRepositoryTests::noNewDirectoriesCountTest() {
    DECLARE_BASIC_MODELS;

#ifdef Q_OS_WIN
    QString filename1 = "C:/path/to/some/file1";
    QString filename2 = "C:/path/to/some/file2";
#else
    QString filename1 = "/path/to/some/file1";
    QString filename2 = "/path/to/some/file2";
#endif

    qint64 dirID = 0;
    artworksRepository.accountFile(filename1, dirID);

    QStringList files;
    files << filename2;

    int newFilesCount = artworksRepository.getNewDirectoriesCount(files);
    QCOMPARE(newFilesCount, 0);
    QCOMPARE(artworksRepository.rowCount(), 1);
}

void ArtworkRepositoryTests::noNewFilesCountTest() {
    DECLARE_BASIC_MODELS;

#ifdef Q_OS_WIN
    QString filename1 = "C:/path/to/some/file1";
    QString filename2 = "C:/path/to/some/file2";
#else
    QString filename1 = "/path/to/some/file1";
    QString filename2 = "/path/to/some/file2";
#endif

    QStringList files;
    files << filename1 << filename2;

    qint64 dirID = 0;
    foreach (const QString &file, files) {
        artworksRepository.accountFile(file, dirID);
    }

    auto filesSource = std::make_shared<Mocks::FilesCollectionMock>(files);
    int newFilesCount = artworksRepository.getNewFilesCount(filesSource);
    QCOMPARE(newFilesCount, 0);
    QCOMPARE(artworksRepository.rowCount(), 1);
}

void ArtworkRepositoryTests::accountFileEmitsTest() {
    DECLARE_BASIC_MODELS;

#ifdef Q_OS_WIN
    QString filename = "C:/path/to/some/file1";
#else
    QString filename = "/path/to/some/file1";
#endif

    QSignalSpy beginSpy(&artworksRepository, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy endSpy(&artworksRepository, SIGNAL(rowsInserted(QModelIndex,int,int)));
    qint64 dirID = 0;
    bool newFiles = artworksRepository.accountFile(filename, dirID);

    QCOMPARE(newFiles, true);

    QCOMPARE(beginSpy.count(), 1);
    QList<QVariant> addArguments = beginSpy.takeFirst();
    QCOMPARE(addArguments.at(1).toInt(), 0);
    QCOMPARE(addArguments.at(2).toInt(), 0);

    QCOMPARE(endSpy.count(), 1);
}

void ArtworkRepositoryTests::selectFolderTest() {
    DECLARE_BASIC_MODELS;

#ifdef Q_OS_WIN
    QString filename1 = "C:/path1/to/some/file";
    QString filename2 = "C:/path2/to/some/file";
    QString filename3 = "C:/path3/to/some/file";
    QString filename4 = "C:/path4/to/some/file";
#else
    QString filename1 = "/path1/to/some/file";
    QString filename2 = "/path2/to/some/file";
    QString filename3 = "/path3/to/some/file";
    QString filename4 = "/path4/to/some/file";
#endif

    QStringList files;
    files << filename1 << filename2 << filename3;

    std::vector<qint64> dirIDs;
    foreach (const QString &file, files) {
        qint64 dirID;
        artworksRepository.accountFile(file, dirID);
        dirIDs.push_back(dirID);
    }

    // Initially all directories are selected.
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[0]), true);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[1]), true);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[2]), true);

    //If All are selected and you click on 1, you select it and deselect others.
    artworksRepository.toggleDirectorySelected(0);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[0]), true);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[1]), false);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[2]), false);

    //If not all are selected and you click on 1, you add it to the selection.
    artworksRepository.toggleDirectorySelected(2);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[0]), true);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[1]), false);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[2]), true);

    artworksRepository.toggleDirectorySelected(0);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[0]), false);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[1]), false);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[2]), true);

    //If you unselect last selected, all get selected.
    artworksRepository.toggleDirectorySelected(2);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[0]), true);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[1]), true);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[2]), true);

    artworksRepository.toggleDirectorySelected(2);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[0]), false);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[1]), false);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[2]), true);

    //If you remove last selected directory, all get selected.
    artworksRepository.removeItem(2);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[0]), true);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[1]), true);

    //If you add a new directory, it gets selected by default.
    qint64 dirID;
    artworksRepository.accountFile(filename4, dirID);
    dirIDs.push_back(dirID);
    QCOMPARE(artworksRepository.isDirectorySelected(dirIDs[3]), true);
}

void ArtworkRepositoryTests::oneEmptyDirectoryStaysTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);

    QCOMPARE(artworksRepository.getFilesCountForDirectory(0), 1);
    QCOMPARE(artworksListModel.getArtworksSize(), count);

    artworksListModel.removeFilesFromDirectory(0);
    QCOMPARE(artworksRepository.rowCount(), 1);

    artworksRepository.cleanupEmptyDirectories();
    QCOMPARE(artworksRepository.rowCount(), 0);
}

void ArtworkRepositoryTests::fewEmptyDirectoriesStayTest() {
    const int count = 2;
    DECLARE_MODELS_AND_GENERATE(count, false);

    QCOMPARE(artworksRepository.getFilesCountForDirectory(0), 1);
    QCOMPARE(artworksRepository.getFilesCountForDirectory(1), 1);
    QCOMPARE(artworksListModel.getArtworksSize(), count);

    artworksListModel.removeFilesFromDirectory(0);
    artworksListModel.removeFilesFromDirectory(1);
    QCOMPARE(artworksRepository.rowCount(), 2);

    artworksRepository.cleanupEmptyDirectories();
    QCOMPARE(artworksRepository.rowCount(), 0);
}

void ArtworkRepositoryTests::allDirsInitiallySelectedTest() {
    SETUP_SELECTION_TEST(10, 3);

    CHECK_ALL_SELECTED;
}

void ArtworkRepositoryTests::unselectOneSelectsOnlyOneTest() {
    SETUP_SELECTION_TEST(10, 3);

    size_t toggleIndex = 0;
    artworksRepository.toggleDirectorySelected(toggleIndex);

    CHECK_ONLY_SELECTED(toggleIndex);
}

void ArtworkRepositoryTests::cannotUnselectTheOnlyOneTest() {
    SETUP_SELECTION_TEST(10, 1);

    artworksRepository.toggleDirectorySelected(0);

    CHECK_ALL_SELECTED;
}

void ArtworkRepositoryTests::toggleSameDirectoryTwiceAllSelectedTest() {
    SETUP_SELECTION_TEST(10, 3);

    size_t toggleIndex = 1;
    artworksRepository.toggleDirectorySelected(toggleIndex);

    CHECK_ONLY_SELECTED(toggleIndex);

    artworksRepository.toggleDirectorySelected(toggleIndex);

    CHECK_ALL_SELECTED;
}

void ArtworkRepositoryTests::unselectLastSelectedSelectsAllTest() {
    SETUP_SELECTION_TEST(10, 3);

    artworksRepository.unselectAllDirectories();
    artworksRepository.accessRepos()[0].setIsSelectedFlag(true);

    artworksRepository.toggleDirectorySelected(0);

    CHECK_ALL_SELECTED;
}

void ArtworkRepositoryTests::removeOnlySelectedSelectsAllTest() {
    SETUP_SELECTION_TEST(10, 3);

    size_t toggleIndex = 0;
    artworksRepository.unselectAllDirectories();
    artworksRepository.accessRepos()[toggleIndex].setIsSelectedFlag(true);

    artworksListModel.removeFilesFromDirectory(0);
    artworksRepository.cleanupEmptyDirectories();

    CHECK_ALL_SELECTED;
}

void ArtworkRepositoryTests::removeOneOfFewSelectedStaysSameTest() {
    SETUP_SELECTION_TEST(10, 3);

    artworksRepository.unselectAllDirectories();
    artworksRepository.accessRepos()[0].setIsSelectedFlag(true);
    artworksRepository.accessRepos()[1].setIsSelectedFlag(true);

    artworksListModel.removeFilesFromDirectory(0);
    artworksRepository.cleanupEmptyDirectories();

    CHECK_SELECTED(0);
    CHECK_UNSELECTED(1);
}

void ArtworkRepositoryTests::undoRemoveOnlySelectedSelectsItTest() {
    SETUP_SELECTION_TEST(10, 3);

    size_t toggleIndex = 2;
    artworksRepository.toggleDirectorySelected(toggleIndex);
    CHECK_ONLY_SELECTED(toggleIndex);

    artworksListModel.removeFilesFromDirectory(toggleIndex);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    CHECK_ONLY_SELECTED(toggleIndex);
}

void ArtworkRepositoryTests::undoRemoveOneOfFewSelectedTest() {
    SETUP_SELECTION_TEST(10, 3);

    CHECK_ALL_SELECTED;

    size_t toggleIndex = 2;
    artworksListModel.removeFilesFromDirectory(toggleIndex);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    CHECK_ALL_SELECTED;
}

void ArtworkRepositoryTests::undoRemoveAfterUserSelectsOtherTest() {
    SETUP_SELECTION_TEST(10, 3);

    size_t toggleIndex = 2;
    artworksRepository.toggleDirectorySelected(toggleIndex);
    CHECK_ONLY_SELECTED(toggleIndex);

    artworksListModel.removeFilesFromDirectory(toggleIndex);

    size_t selectedIndex = 0;
    artworksRepository.toggleDirectorySelected(selectedIndex);
    CHECK_ONLY_SELECTED(selectedIndex);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    CHECK_SELECTED(selectedIndex);
    CHECK_UNSELECTED(1);
    CHECK_SELECTED(toggleIndex);
}

void ArtworkRepositoryTests::undoRemoveAfterUserSelectsFewTest() {
    SETUP_SELECTION_TEST(10, 4);

    size_t toggleIndex = 3;
    artworksRepository.toggleDirectorySelected(toggleIndex);
    CHECK_ONLY_SELECTED(toggleIndex);

    artworksListModel.removeFilesFromDirectory(toggleIndex);

    CHECK_ALL_SELECTED;

    artworksRepository.toggleDirectorySelected(0);
    CHECK_ONLY_SELECTED(0);

    artworksRepository.toggleDirectorySelected(1);
    CHECK_SELECTED(0);
    CHECK_SELECTED(1);
    CHECK_UNSELECTED(2);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    CHECK_SELECTED(0);
    CHECK_SELECTED(1);
    CHECK_UNSELECTED(2);
    CHECK_SELECTED(3); // toggleIndex
}

void ArtworkRepositoryTests::undoRemoveOfTheOnlyOneSelectsItTest() {
    SETUP_SELECTION_TEST(10, 1);

    CHECK_ALL_SELECTED;

    artworksListModel.removeFilesFromDirectory(0);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    CHECK_ALL_SELECTED;
}

void ArtworkRepositoryTests::undoRemoveAfterAllOtherSelectedTest() {
    SETUP_SELECTION_TEST(10, 4);

    size_t toggleIndex = 3;
    artworksRepository.toggleDirectorySelected(toggleIndex);
    CHECK_ONLY_SELECTED(toggleIndex);

    artworksListModel.removeFilesFromDirectory(toggleIndex);

    CHECK_ALL_SELECTED;

    artworksRepository.toggleDirectorySelected(0);
    CHECK_ONLY_SELECTED(0);

    artworksRepository.toggleDirectorySelected(1);
    CHECK_SELECTED(0);
    CHECK_SELECTED(1);
    CHECK_UNSELECTED(2);

    artworksRepository.toggleDirectorySelected(2);
    CHECK_SELECTED(0);
    CHECK_SELECTED(1);
    CHECK_SELECTED(2);

    bool undoStatus = undoRedoManager.undoLastAction();
    QVERIFY(undoStatus);

    CHECK_ALL_SELECTED;
}
