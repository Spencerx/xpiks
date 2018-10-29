#include "artworkproxy_tests.h"

#include <memory>

#include <QStringList>

#include "KeywordsPresets/presetkeywordsmodel.h"
#include "Models/Editing/artworkproxymodel.h"
#include "Models/Session/recentdirectoriesmodel.h"
#include "UndoRedo/undoredomanager.h"

#include "Mocks/artworkmetadatamock.h"
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/artworksrepositorymock.h"
#include "Mocks/artworksupdatermock.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/coretestsenvironment.h"

#define DECLARE_MODELS_AND_GENERATE(count, withVector) \
    Mocks::CoreTestsEnvironment environment; \
    Models::RecentDirectoriesModel recentDirectories(environment);\
    recentDirectories.initialize();\
    UndoRedo::UndoRedoManager undoRedoManager;\
    Mocks::CommandManagerMock commandManager(undoRedoManager); \
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories);\
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository);\
    artworksListModel.generateAndAddArtworks(count, withVector);\
    Mocks::ArtworksUpdaterMock updater;\
    KeywordsPresets::PresetKeywordsModel presetKeywordsModel(environment); \
    Models::ArtworkProxyModel artworkProxyModel(commandManager, presetKeywordsModel, updater);

void ArtworkProxyTests::editKeywordsPropagatesToArtworkTest() {
    DECLARE_MODELS_AND_GENERATE(5, false);
    artworkProxyModel.setSourceArtwork(artworksListModel.getMockArtwork(0), 0);

    const QString keywordToAdd = "custom keyword";
    artworkProxyModel.appendKeyword(keywordToAdd);
    QVERIFY(artworksListModel.getMockArtwork(0)->hasKeywords(QStringList() << keywordToAdd));
}

void ArtworkProxyTests::editTitlePropagatesToArtworkTest() {
    DECLARE_MODELS_AND_GENERATE(5, false);
    artworkProxyModel.setSourceArtwork(artworksListModel.getMockArtwork(0), 0);

    const QString title = "custom Title";
    artworkProxyModel.setTitle(title);
    QCOMPARE(artworksListModel.getMockArtwork(0)->getTitle(), title);
}

void ArtworkProxyTests::editDescriptionPropagatesToArtworkTest() {
    DECLARE_MODELS_AND_GENERATE(5, false);
    artworkProxyModel.setSourceArtwork(artworksListModel.getMockArtwork(0), 0);

    const QString description = "custom Description";
    artworkProxyModel.setDescription(description);
    QCOMPARE(artworksListModel.getMockArtwork(0)->getDescription(), description);
}
