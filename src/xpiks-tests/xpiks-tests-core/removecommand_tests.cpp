#include <QStringList>
#include <QPair>
#include <QSignalSpy>
#include "removecommand_tests.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/artworksrepositorymock.h"
#include "Mocks/coretestsenvironment.h"
#include "Mocks/selectedindicessourcemock.h"
#include <Commands/Files/removefilescommand.h>
#include <Models/Artworks/artworksrepository.h>
#include "Models/Session/recentdirectoriesmodel.h"

#define DECLARE_MODELS \
    Mocks::CoreTestsEnvironment environment;\
    Models::RecentDirectoriesModel recentDirectories(environment);\
    recentDirectories.initialize();\
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories);\
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository);

void RemoveCommandTests::removeArtworksFromEmptyRepository() {
    DECLARE_MODELS;

    Helpers::IndicesRanges ranges({0, 1, 2});
    auto removeCommand = std::make_shared<Commands::RemoveFilesCommand>(
                             ranges,
                             artworksListModel,
                             artworksRepository);

    QSignalSpy rowsRemovedItemsStart(&artworksListModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy rowsRemovedItemsEnd(&artworksListModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QSignalSpy rowsRemovedRepositoryStart(&artworksRepository, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy rowsRemovedRepositoryEnd(&artworksRepository, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QSignalSpy dataChangedInRepository(&artworksRepository, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

    QSignalSpy modifiedFilesChanged(&artworksListModel, SIGNAL(modifiedArtworksCountChanged()));

    removeCommand->execute();

    int artworksRemovedCount = removeCommand->getRemovedCount();

    QCOMPARE(artworksRemovedCount, 0);

    QCOMPARE(rowsRemovedItemsStart.count(), 0);
    QCOMPARE(rowsRemovedItemsEnd.count(), 0);

    QCOMPARE(rowsRemovedRepositoryStart.count(), 0);
    QCOMPARE(rowsRemovedRepositoryEnd.count(), 0);

    QCOMPARE(dataChangedInRepository.count(), 0);
    QCOMPARE(modifiedFilesChanged.count(), 0);
}

void RemoveCommandTests::removeAllArtworksFromRepository() {
    DECLARE_MODELS;

    int itemsToAdd = 5;
    artworksListModel.generateAndAddArtworks(itemsToAdd);

    int dirsCount = artworksRepository.rowCount();

    Helpers::IndicesRanges ranges(itemsToAdd);
    auto removeCommand = std::make_shared<Commands::RemoveFilesCommand>(
                             ranges,
                             artworksListModel,
                             artworksRepository);

    QSignalSpy rowsRemovedItemsStart(&artworksListModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy rowsRemovedItemsEnd(&artworksListModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QSignalSpy rowsRemovedRepositoryStart(&artworksRepository, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy rowsRemovedRepositoryEnd(&artworksRepository, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QSignalSpy dataChangedInRepository(&artworksRepository, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

    QSignalSpy modifiedFilesChanged(&artworksListModel, SIGNAL(modifiedArtworksCountChanged()));

    removeCommand->execute();
    artworksListModel.deleteRemovedItems();

    int artworksRemovedCount = removeCommand->getRemovedCount();

    QCOMPARE(artworksRemovedCount, itemsToAdd);

    QCOMPARE(rowsRemovedItemsStart.count(), 1);
    QList<QVariant> rowsRemovedSpyArguments = rowsRemovedItemsStart.takeFirst();
    QCOMPARE(rowsRemovedSpyArguments.at(1).toInt(), 0);
    QCOMPARE(rowsRemovedSpyArguments.at(2).toInt(), itemsToAdd - 1);

    QCOMPARE(rowsRemovedRepositoryStart.count(), 0);
    QCOMPARE(rowsRemovedRepositoryEnd.count(), 0);

    QCOMPARE(rowsRemovedItemsEnd.count(), 1);
    QList<QVariant> rowsRemovedFinishedSpyArguments = rowsRemovedItemsEnd.takeFirst();
    QCOMPARE(rowsRemovedFinishedSpyArguments.at(1).toInt(), 0);
    QCOMPARE(rowsRemovedFinishedSpyArguments.at(2).toInt(), itemsToAdd - 1);

    QCOMPARE(dataChangedInRepository.count(), 1);
    QList<QVariant> dataChangedSpyArguments = dataChangedInRepository.takeFirst();
    QCOMPARE(dataChangedSpyArguments.at(1).toInt(), 0);
    // no more directories and rowCount() == 0 in event raising
    QCOMPARE(dataChangedSpyArguments.at(2).toInt(), 0);

    QCOMPARE(modifiedFilesChanged.count(), 1);

    // ------

    artworksRepository.cleanupEmptyDirectories();

    QCOMPARE(rowsRemovedRepositoryStart.count(), 1);
    QList<QVariant> repositoriesRemovedSpyArguments = rowsRemovedRepositoryStart.takeFirst();
    QCOMPARE(repositoriesRemovedSpyArguments.at(1).toInt(), 0);
    QCOMPARE(repositoriesRemovedSpyArguments.at(2).toInt(), dirsCount - 1);

    QCOMPARE(rowsRemovedRepositoryEnd.count(), 1);
    QList<QVariant> repositoriesRemovedFinishedSpyArguments = rowsRemovedRepositoryEnd.takeFirst();
    QCOMPARE(repositoriesRemovedFinishedSpyArguments.at(1).toInt(), 0);
    QCOMPARE(repositoriesRemovedFinishedSpyArguments.at(2).toInt(), dirsCount - 1);
}
