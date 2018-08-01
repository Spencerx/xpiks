#include <QStringList>
#include <QSignalSpy>
#include "addcommand_tests.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/artworksrepositorymock.h"
#include "Mocks/coretestsenvironment.h"
#include <Commands/Files/addfilescommand.h>
#include <Models/Session/recentdirectoriesmodel.h>
#include <Mocks/filescollectionmock.h>

#define DECLARE_MODELS \
    Mocks::CoreTestsEnvironment environment;\
    Models::RecentDirectoriesModel recentDirectories(environment);\
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories);\
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository);

void AddCommandTests::addNoArtworksToEmptyRepositoryTest() {
    DECLARE_MODELS;

    QSignalSpy artItemsBeginInsertSpy(&artworksListModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy artItemsEndInsertSpy(&artworksListModel, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy artworkRepoBeginInsertSpy(&artworksRepository, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy artworkRepoEndInsertSpy(&artworksRepository, SIGNAL(rowsInserted(QModelIndex,int,int)));

    QStringList filenames;
    auto filesCollection = std::make_shared<Mocks::FilesCollectionMock>(filenames);
    auto addFilesCommand = std::make_shared<Commands::AddFilesCommand>(filesCollection,
                                                                       Common::AddFilesFlags::None,
                                                                       artworksListModel);
    addFilesCommand->execute();
    int newFilesCount = addFilesCommand->getAddedCount();

    QCOMPARE(newFilesCount, 0);

    QCOMPARE(artItemsBeginInsertSpy.count(), 0);
    QCOMPARE(artItemsEndInsertSpy.count(), 0);
    QCOMPARE(artworkRepoBeginInsertSpy.count(), 0);
    QCOMPARE(artworkRepoEndInsertSpy.count(), 0);
}

void AddCommandTests::addOneArtworkToEmptyRepositoryTest() {
    DECLARE_MODELS;

    QSignalSpy artItemsBeginInsertSpy(&artworksListModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy artItemsEndInsertSpy(&artworksListModel, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy artworkRepoBeginInsertSpy(&artworksRepository, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy artworkRepoEndInsertSpy(&artworksRepository, SIGNAL(rowsInserted(QModelIndex,int,int)));

    QStringList filenames;
    filenames.append("somefile.jpg");

    auto filesCollection = std::make_shared<Mocks::FilesCollectionMock>(filenames);
    auto addFilesCommand = std::make_shared<Commands::AddFilesCommand>(filesCollection,
                                                                       Common::AddFilesFlags::None,
                                                                       artworksListModel);
    addFilesCommand->execute();
    int newFilesCount = addFilesCommand->getAddedCount();

    QCOMPARE(newFilesCount, 1);

    QCOMPARE(artItemsBeginInsertSpy.count(), 1);
    QList<QVariant> artItemsSpyArguments = artItemsBeginInsertSpy.takeFirst();
    QCOMPARE(artItemsSpyArguments.at(1).toInt(), 0);
    QCOMPARE(artItemsSpyArguments.at(2).toInt(), 0);

    QCOMPARE(artItemsEndInsertSpy.count(), 1);
    artItemsSpyArguments = artItemsEndInsertSpy.takeFirst();
    QCOMPARE(artItemsSpyArguments.at(1).toInt(), 0);
    QCOMPARE(artItemsSpyArguments.at(2).toInt(), 0);

    QCOMPARE(artworkRepoBeginInsertSpy.count(), 1);
    QList<QVariant> artworkRepSpyArgs = artworkRepoBeginInsertSpy.takeFirst();
    QCOMPARE(artworkRepSpyArgs.at(1).toInt(), 0);
    QCOMPARE(artworkRepSpyArgs.at(2).toInt(), 0);

    QCOMPARE(artworkRepoEndInsertSpy.count(), 1);
    artworkRepSpyArgs = artworkRepoEndInsertSpy.takeFirst();
    QCOMPARE(artworkRepSpyArgs.at(1).toInt(), 0);
    QCOMPARE(artworkRepSpyArgs.at(2).toInt(), 0);
}

void AddCommandTests::addAndAttachVectorsTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile.jpg" << "/another/path/to/some/other/file.jpg";
    vectors << "/path/to/somefile.eps" << "/another/path/to/some/other/file.eps";

    auto filesCollection = std::make_shared<Mocks::FilesCollectionMock>(filenames, vectors);
    auto addFilesCommand = std::make_shared<Commands::AddFilesCommand>(filesCollection,
                                                                       Common::AddFilesFlags::None,
                                                                       artworksListModel);
    addFilesCommand->execute();
    int newFilesCount = addFilesCommand->getAddedCount();

    QCOMPARE(newFilesCount, 2);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(artworksListModel.getMockArtwork(i)->hasVectorAttached());
    }
}

void AddCommandTests::addAndAttachVectorsLaterTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile.jpg" << "/another/path/to/some/other/file.jpg";
    vectors << "/path/to/somefile.eps" << "/another/path/to/some/other/file.eps";

    auto filesCollection = std::make_shared<Mocks::FilesCollectionMock>(filenames);
    auto addFilesCommand = std::make_shared<Commands::AddFilesCommand>(filesCollection,
                                                                       Common::AddFilesFlags::None,
                                                                       artworksListModel);
    addFilesCommand->execute();
    int newFilesCount = addFilesCommand->getAddedCount();

    QCOMPARE(newFilesCount, 2);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(!artworksListModel.getMockArtwork(i)->hasVectorAttached());
    }

    filesCollection = std::make_shared<Mocks::FilesCollectionMock>(QStringList(), vectors);
    addFilesCommand = std::make_shared<Commands::AddFilesCommand>(filesCollection,
                                                                       Common::AddFilesFlags::None,
                                                                       artworksListModel);
    addFilesCommand->execute();
    newFilesCount = addFilesCommand->getAddedCount();

    QCOMPARE(newFilesCount, 0);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(artworksListModel.getMockArtwork(i)->hasVectorAttached());
    }
}

void AddCommandTests::addAndDontAttachVectorsOtherDirTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile.jpg" << "/another/path/to/some/other/file.jpg";
    vectors << "/another/path/to/somefile.eps" << "/path/to/some/other/file.eps";

    auto filesCollection = std::make_shared<Mocks::FilesCollectionMock>(filenames, vectors);
    auto addFilesCommand = std::make_shared<Commands::AddFilesCommand>(filesCollection,
                                                                       Common::AddFilesFlags::None,
                                                                       artworksListModel);
    addFilesCommand->execute();
    int newFilesCount = addFilesCommand->getAddedCount();

    QCOMPARE(newFilesCount, 2);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(!artworksListModel.getMockArtwork(i)->hasVectorAttached());
    }
}

void AddCommandTests::addAndDontAttachVectorsEmptyDirTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile.jpg" << "/another/path/to/some/other/file.jpg";
    vectors << "somefile.eps" << "file.eps";

    auto filesCollection = std::make_shared<Mocks::FilesCollectionMock>(filenames, vectors);
    auto addFilesCommand = std::make_shared<Commands::AddFilesCommand>(filesCollection,
                                                                       Common::AddFilesFlags::None,
                                                                       artworksListModel);
    addFilesCommand->execute();
    int newFilesCount = addFilesCommand->getAddedCount();

    QCOMPARE(newFilesCount, 2);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(!artworksListModel.getMockArtwork(i)->hasVectorAttached());
    }
}

void AddCommandTests::addAndDontAttachVectorsStartsWithTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile.jpg" << "/another/path/to/some/other/file.jpg";
    vectors << "/to/somefile.eps" << "/path/to/some/other/file.eps";

    auto filesCollection = std::make_shared<Mocks::FilesCollectionMock>(filenames, vectors);
    auto addFilesCommand = std::make_shared<Commands::AddFilesCommand>(filesCollection,
                                                                       Common::AddFilesFlags::None,
                                                                       artworksListModel);
    addFilesCommand->execute();
    int newFilesCount = addFilesCommand->getAddedCount();

    QCOMPARE(newFilesCount, 2);

    for (int i = 0; i < 2; ++i) {
        QVERIFY(!artworksListModel.getMockArtwork(i)->hasVectorAttached());
    }
}

void AddCommandTests::addAndAttachFromSingleDirectoryTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile1.jpg" << "/path/to/somefile2.jpg" << "/another/path/to/somefile1.jpg" << "/another/path/to/somefile2.jpg";
    vectors << "/path/to/somefile1.eps" << "/path/to/somefile2.eps" << "/another/path/to/somefile1.eps" << "/another/path/to/somefile2.eps";

    auto filesCollection = std::make_shared<Mocks::FilesCollectionMock>(filenames, vectors);
    auto addFilesCommand = std::make_shared<Commands::AddFilesCommand>(filesCollection,
                                                                       Common::AddFilesFlags::None,
                                                                       artworksListModel);
    addFilesCommand->execute();
    int newFilesCount = addFilesCommand->getAddedCount();

    QCOMPARE(newFilesCount, filenames.length());

    for (int i = 0; i < filenames.length(); ++i) {
        QVERIFY(artworksListModel.getMockArtwork(i)->hasVectorAttached());
    }
}

void AddCommandTests::addSingleDirectoryAndAttachLaterTest() {
    DECLARE_MODELS;

    QStringList filenames, vectors;
    filenames << "/path/to/somefile1.jpg" << "/path/to/somefile2.jpg" << "/another/path/to/somefile1.jpg" << "/another/path/to/somefile2.jpg";
    vectors << "/path/to/somefile1.eps" << "/path/to/somefile2.eps" << "/another/path/to/somefile1.eps" << "/another/path/to/somefile2.eps";

    auto filesCollection = std::make_shared<Mocks::FilesCollectionMock>(filenames, vectors);
    auto addFilesCommand = std::make_shared<Commands::AddFilesCommand>(filesCollection,
                                                                       Common::AddFilesFlags::None,
                                                                       artworksListModel);
    addFilesCommand->execute();
    int newFilesCount = addFilesCommand->getAddedCount();

    QCOMPARE(newFilesCount, filenames.length());

    for (int i = 0; i < filenames.length(); ++i) {
        QVERIFY(!artworksListModel.getMockArtwork(i)->hasVectorAttached());
    }

    filesCollection = std::make_shared<Mocks::FilesCollectionMock>(QStringList(), vectors);
    addFilesCommand = std::make_shared<Commands::AddFilesCommand>(filesCollection,
                                                                       Common::AddFilesFlags::None,
                                                                       artworksListModel);
    addFilesCommand->execute();
    newFilesCount = addFilesCommand->getAddedCount();

    QCOMPARE(newFilesCount, 0);

    for (int i = 0; i < filenames.length(); ++i) {
        QVERIFY(artworksListModel.getMockArtwork(i)->hasVectorAttached());
    }
}
