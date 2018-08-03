#include "removefilesfs_tests.h"
#include <QStringList>
#include <QPair>
#include <QTest>
#include <QSignalSpy>
#include "Mocks/commandmanagermock.h"
#include "Mocks/artworksrepositorymock.h"
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/coretestsenvironment.h"
#include <Models/Artworks/artworksrepository.h>
#include <Artworks/artworkelement.h>
#include <Models/Editing/combinedartworksmodel.h>
#include <Models/Connectivity/ziparchiver.h>
#include <Models/settingsmodel.h>

#define DECLARE_MODELS_AND_GENERATE_(count) \
    Mocks::CoreTestsEnvironment environment; \
    UndoRedo::UndoRedoManager undoRedoManager; \
    Mocks::CommandManagerMock commandManager(undoRedoManager); \
    Models::RecentDirectoriesModel recentDirectories(environment);\
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories);\
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository);\
    KeywordsPresets::PresetKeywordsModel keywordsPresets(environment);\
    Models::SettingsModel settingsModel(environment);\
    Models::FilteredArtworksListModel filteredArtworksModel(\
    artworksListModel, commandManager, keywordsPresets, settingsModel);\
    Models::CombinedArtworksModel combinedModel(commandManager, keywordsPresets); \
    Models::ZipArchiver zipArchiver; \
    artworksListModel.generateAndAddArtworks(10);

void RemoveFilesFsTests::removeArtworksSignals() {
    Mocks::CommandManagerMock commandManagerMock;
    Mocks::ArtworksListModelMock artItemsMock;
    Mocks::CoreTestsEnvironment environment;
    Mocks::ArtworksRepositoryMock artworksRepositoryMock(environment);

    Models::ArtworksRepository *artworksRepository = &artworksRepositoryMock;
    Models::ArtItemsModel *artItemsModel = &artItemsMock;
    commandManagerMock.InjectDependency(artItemsModel);
    commandManagerMock.InjectDependency(artworksRepository);

    int itemsToAdd = 5;
    QSignalSpy artRepositoryFileDeleted(artworksRepository, SIGNAL(filesUnavailable()));
    QSignalSpy ArtItemFileDeleted(artItemsModel, SIGNAL(unavailableArtworksFound()));

    commandManagerMock.generateAndAddArtworks(itemsToAdd);

    artworksRepositoryMock.removeFileAndEmitSignal();

// signals
    QCOMPARE(artRepositoryFileDeleted.count(), 1);
    QCOMPARE(ArtItemFileDeleted.count(), 1);
}

void RemoveFilesFsTests::removeArtworksNumberItems() {
    int itemsToAdd = 10, itemsToDelete = 5;
    DECLARE_MODELS_AND_GENERATE_(itemsToAdd);

    combinedModel.resetModel();

    MetadataIO::WeakArtworksSnapshot artworksList;

    for (int i = 0; i < itemsToAdd; i++) {
         auto *metadata = ArtworksListModelMock.getMockArtwork(i);
         artworksList.push_back(metadata);
    }

    combinedModel.resetModel();
    combinedModel.setArtworks(artworksList);

    Artworks::ArtworksSnapshot snapshot(artworksList);
    zipArchive.setArtworks(snapshot);

// delete
   artworksListModel.mockDeletion(itemsToDelete);

// send accept
    commandManagerMock.mockAcceptDeletion();

//items
    QCOMPARE(ArtworksListModelMock.getArtworksCount(), itemsToAdd - itemsToDelete);
    QCOMPARE(combinedModel.getArtworksCount(), itemsToAdd - itemsToDelete);
    QCOMPARE(filteredItemsModel.getItemsCount(), itemsToAdd - itemsToDelete);
    QCOMPARE(zipArchive.getItemsCount(), itemsToAdd - itemsToDelete);
}

void RemoveFilesFsTests::removeArtworksAllItems() {
    int itemsToAdd = 10, itemsToDelete = 10;
    DECLARE_MODELS_AND_GENERATE_(itemsToAdd);

    combinedModel.resetModel();
    MetadataIO::WeakArtworksSnapshot artworksList;

    for (int i = 0; i < itemsToAdd; i++) {
         auto *metadata = ArtworksListModelMock.getMockArtwork(i);
         artworksList.push_back(metadata);
    }

    combinedModel.resetModel();
    combinedModel.setArtworks(artworksList);

    Artworks::ArtworksSnapshot snapshot(artworksList);
    zipArchive.setArtworks(snapshot);

// delete
   commandManagerMock.mockDeletion(itemsToDelete);

// send accept
    commandManagerMock.mockAcceptDeletion();

//items
    QCOMPARE(ArtworksListModelMock.getArtworksCount(), itemsToAdd - itemsToDelete);
    QCOMPARE(combinedModel.getArtworksCount(), itemsToAdd - itemsToDelete);
    QCOMPARE(filteredItemsModel.getItemsCount(), itemsToAdd - itemsToDelete);
    QCOMPARE(zipArchive.getItemsCount(), itemsToAdd - itemsToDelete);
}

void RemoveFilesFsTests::removeVectorSmokeTest() {
    Mocks::CommandManagerMock commandManagerMock;
    Mocks::ArtworksListModelMock artItemsMock;
    Mocks::CoreTestsEnvironment environment;
    Mocks::ArtworksRepositoryMock artworksRepositoryMock(environment);

    Models::ArtworksRepository *artworksRepository = &artworksRepositoryMock;
    Models::ArtItemsModel *artItemsModel = &artItemsMock;
    commandManagerMock.InjectDependency(artItemsModel);
    commandManagerMock.InjectDependency(artworksRepository);

    const int itemsToAdd = 5;
    QSignalSpy artRepositoryFileDeleted(artworksRepository, SIGNAL(filesUnavailable()));
    QSignalSpy vectorRemovedSpy(artItemsModel, SIGNAL(unavailableVectorsFound()));
    QSignalSpy artworkRemovedSpy(artItemsModel, SIGNAL(unavailableArtworksFound()));

    commandManagerMock.generateAndAddArtworks(itemsToAdd);

    QObject::connect(artworksRepository, &Models::ArtworksRepository::filesUnavailable,
                     artItemsModel, &Models::ArtItemsModel::onFilesUnavailableHandler);

    artworksRepositoryMock.removeVectorAndEmitSignal();

// signals
    QCOMPARE(artRepositoryFileDeleted.count(), 1);
    QCOMPARE(vectorRemovedSpy.count(), 1);
    QCOMPARE(artworkRemovedSpy.count(), 0);
}
