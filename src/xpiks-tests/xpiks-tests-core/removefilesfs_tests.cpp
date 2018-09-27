#include "removefilesfs_tests.h"
#include <QStringList>
#include <QPair>
#include <QTest>
#include <QSignalSpy>
#include "Mocks/commandmanagermock.h"
#include "Mocks/artworksrepositorymock.h"
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/coretestsenvironment.h"
#include "Mocks/artworksupdatermock.h"
#include <Artworks/artworkelement.h>
#include <Artworks/artworkssnapshot.h>
#include <UndoRedo/undoredomanager.h>
#include <KeywordsPresets/presetkeywordsmodel.h>
#include <Models/Artworks/artworksrepository.h>
#include <Models/Editing/combinedartworksmodel.h>
#include <Models/Connectivity/ziparchiver.h>
#include <Models/settingsmodel.h>
#include <Models/Session/recentdirectoriesmodel.h>
#include <Models/Artworks/filteredartworkslistmodel.h>

#define DECLARE_BASIC_MODELS\
    Mocks::CoreTestsEnvironment environment; \
    UndoRedo::UndoRedoManager undoRedoManager; \
    Mocks::CommandManagerMock commandManager(undoRedoManager); \
    Models::RecentDirectoriesModel recentDirectories(environment);\
    recentDirectories.initialize();\
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories);\
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository);

#define DECLARE_MODELS_AND_GENERATE_(count) \
    DECLARE_BASIC_MODELS\
    KeywordsPresets::PresetKeywordsModel keywordsPresets(environment);\
    Models::SettingsModel settingsModel(environment);\
    settingsModel.initializeConfigs();\
    Models::FilteredArtworksListModel filteredArtworksModel(\
    artworksListModel, commandManager, keywordsPresets, settingsModel);\
    Mocks::ArtworksUpdaterMock updater;\
    Models::CombinedArtworksModel combinedModel(updater, keywordsPresets); \
    Models::ZipArchiver zipArchiver; \
    artworksListModel.generateAndAddArtworks(10);\
    Common::connectSource<Common::NamedType<int, Common::MessageType::UnavailableFiles>>(artworksListModel,\
{zipArchiver, combinedModel});

void RemoveFilesFsTests::removeArtworksSignals() {
    DECLARE_BASIC_MODELS;
    artworksListModel.generateAndAddArtworks(5);

    QSignalSpy artRepositoryFileDeleted(&artworksRepository, SIGNAL(filesUnavailable()));
    QSignalSpy artworksListFileDeleted(&artworksListModel, SIGNAL(unavailableArtworksFound()));

    artworksRepository.removeFileAndEmitSignal();

    // signals
    QCOMPARE(artRepositoryFileDeleted.count(), 1);
    QCOMPARE(artworksListFileDeleted.count(), 1);
}

void RemoveFilesFsTests::removeArtworksNumberItems() {
    int itemsToAdd = 10, itemsToDelete = 5;
    DECLARE_MODELS_AND_GENERATE_(itemsToAdd);

    Artworks::ArtworksSnapshot snapshot = artworksListModel.createArtworksSnapshot();

    combinedModel.setArtworks(snapshot);
    zipArchiver.setArtworks(snapshot);

    // delete
    artworksListModel.mockDeletion(itemsToDelete);

    // send accept
    artworksListModel.purgeUnavailableFiles();

    //items
    QCOMPARE((int)artworksListModel.getArtworksSize(), itemsToAdd - itemsToDelete);
    QCOMPARE(combinedModel.getArtworksCount(), itemsToAdd - itemsToDelete);
    QCOMPARE(filteredArtworksModel.getItemsCount(), itemsToAdd - itemsToDelete);
    QCOMPARE(zipArchiver.getItemsCount(), itemsToAdd - itemsToDelete);
}

void RemoveFilesFsTests::removeArtworksAllItems() {
    int itemsToAdd = 10, itemsToDelete = 10;
    DECLARE_MODELS_AND_GENERATE_(itemsToAdd);

    Artworks::ArtworksSnapshot snapshot;

    for (int i = 0; i < itemsToAdd; i++) {
        auto artwork = artworksListModel.getMockArtwork(i);
        snapshot.append(artwork);
    }

    combinedModel.setArtworks(snapshot);
    zipArchiver.setArtworks(snapshot);

    // delete
    artworksListModel.mockDeletion(itemsToDelete);

    artworksListModel.purgeUnavailableFiles();

    //items
    QCOMPARE((int)artworksListModel.getArtworksSize(), itemsToAdd - itemsToDelete);
    QCOMPARE(combinedModel.getArtworksCount(), itemsToAdd - itemsToDelete);
    QCOMPARE(filteredArtworksModel.getItemsCount(), itemsToAdd - itemsToDelete);
    QCOMPARE(zipArchiver.getItemsCount(), itemsToAdd - itemsToDelete);
}

void RemoveFilesFsTests::removeVectorSmokeTest() {
    DECLARE_BASIC_MODELS;

    QSignalSpy artRepositoryFileDeleted(&artworksRepository, SIGNAL(filesUnavailable()));
    QSignalSpy vectorRemovedSpy(&artworksListModel, SIGNAL(unavailableVectorsFound()));
    QSignalSpy artworkRemovedSpy(&artworksListModel, SIGNAL(unavailableArtworksFound()));

    artworksListModel.generateAndAddArtworks(5);

    artworksRepository.removeVectorAndEmitSignal();

    // signals
    QCOMPARE(artRepositoryFileDeleted.count(), 1);
    QCOMPARE(vectorRemovedSpy.count(), 1);
    QCOMPARE(artworkRemovedSpy.count(), 0);
}
