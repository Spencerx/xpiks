#include "deletekeywords_tests.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/artworkmetadatamock.h"
#include "Mocks/coretestsenvironment.h"
#include "Mocks/artworksrepositorymock.h"
#include <Models/Editing/deletekeywordsviewmodel.h>
#include <Models/Artworks/artworksrepository.h>
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Models/Session/recentdirectoriesmodel.h>
#include <KeywordsPresets/presetkeywordsmodel.h>
#include <UndoRedo/undoredomanager.h>

#define DECLARE_MODELS_AND_GENERATE(count) \
    Mocks::CoreTestsEnvironment environment; \
    Models::RecentDirectoriesModel recentDirectories(environment);\
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories); \
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository); \
    UndoRedo::UndoRedoManager undoRedoManager;\
    Mocks::CommandManagerMock commandManager(undoRedoManager); \
    KeywordsPresets::PresetKeywordsModel keywordsPresets(environment);\
    Models::DeleteKeywordsViewModel deleteKeywordsModel(commandManager, keywordsPresets); \
    artworksListModel.generateAndAddArtworks(count);

void DeleteKeywordsTests::smokeTest() {
    DECLARE_MODELS_AND_GENERATE(2);

    const QString keywordToDelete = "keyword1";
    QStringList keywords;
    keywords << keywordToDelete << "keyword2" << "keyword3";

    artworksListModel.foreachArtwork([&keywords](int, Mocks::ArtworkMetadataMock *artwork) {
        artwork->setKeywords(keywords);
    });

    deleteKeywordsModel.setArtworks(artworksListModel.createArtworksSnapshot());
    deleteKeywordsModel.appendKeywordToDelete(keywordToDelete);
    deleteKeywordsModel.deleteKeywords();

    artworksListModel.foreachArtwork([&](int, Mocks::ArtworkMetadataMock *artwork) {
        QCOMPARE(artwork->rowCount(), keywords.length() - 1);
        auto *keywordsModel = artwork->getBasicModel();
        QVERIFY(!keywordsModel->containsKeyword(keywordToDelete));
    });
}

void DeleteKeywordsTests::keywordsCombinedTest() {
    DECLARE_MODELS_AND_GENERATE(3);

    const QString keywordToDelete = "keywordToDelete";

    artworksListModel.foreachArtwork([&](int index, Mocks::ArtworkMetadataMock *artwork) {
        artwork->clearKeywords();
        artwork->appendKeyword("keyword" + QString::number(index));
        artwork->appendKeyword(keywordToDelete);
    });

    deleteKeywordsModel.setArtworks(artworksListModel.createArtworksSnapshot());

    QVERIFY(deleteKeywordsModel.containsCommonKeyword(keywordToDelete));

    deleteKeywordsModel.appendKeywordToDelete(keywordToDelete);
    deleteKeywordsModel.deleteKeywords();

    artworksListModel.foreachArtwork([&](int, Mocks::ArtworkMetadataMock *artwork) {
        auto *keywordsModel = artwork->getBasicModel();
        QVERIFY(!keywordsModel->containsKeyword(keywordToDelete));
    });
}

void DeleteKeywordsTests::doesNotDeleteOtherCaseTest() {
    DECLARE_MODELS_AND_GENERATE(3);

    const QString keywordToDelete = "keywordToDelete";

    artworksListModel.foreachArtwork([&](int index, Mocks::ArtworkMetadataMock *artwork) {
        artwork->clearKeywords();
        artwork->appendKeyword("keyword" + QString::number(index));
        artwork->appendKeyword(keywordToDelete);
    });

    deleteKeywordsModel.setArtworks(artworksListModel.createArtworksSnapshot());
    deleteKeywordsModel.appendKeywordToDelete(keywordToDelete.toUpper());
    deleteKeywordsModel.setCaseSensitive(true);
    deleteKeywordsModel.deleteKeywords();

    artworksListModel.foreachArtwork([&](int, Mocks::ArtworkMetadataMock *artwork) {
        auto *keywordsModel = artwork->getBasicModel();
        QCOMPARE(keywordsModel->getKeywordsCount(), 2);
        QVERIFY(keywordsModel->containsKeyword(keywordToDelete));
    });
}

void DeleteKeywordsTests::doesNotDeleteNoKeywordsTest() {
    DECLARE_MODELS_AND_GENERATE(3);

    const QString keywordToDelete = "keywordToDelete";

    artworksListModel.foreachArtwork([&](int index, Mocks::ArtworkMetadataMock *artwork) {
        artwork->clearKeywords();
        artwork->appendKeyword("keyword" + QString::number(index));
        artwork->appendKeyword(keywordToDelete);
    });

    deleteKeywordsModel.setArtworks(artworksListModel.createArtworksSnapshot());
    deleteKeywordsModel.deleteKeywords();

    artworksListModel.foreachArtwork([&](int, Mocks::ArtworkMetadataMock *artwork) {
        auto *keywordsModel = artwork->getBasicModel();
        QCOMPARE(keywordsModel->getKeywordsCount(), 2);
        QVERIFY(keywordsModel->containsKeyword(keywordToDelete));
    });
}

void DeleteKeywordsTests::deleteCaseInsensitiveTest() {
    DECLARE_MODELS_AND_GENERATE(5);

    const QString keywordToDelete = "keywordToDelete";

    artworksListModel.foreachArtwork([&](int index, Mocks::ArtworkMetadataMock *artwork) {
        artwork->clearKeywords();
        artwork->appendKeyword("keyword" + QString::number(index));
        artwork->appendKeyword(index % 2 == 0 ? keywordToDelete.toLower() : keywordToDelete.toUpper());
    });

    deleteKeywordsModel.setArtworks(artworksListModel.createArtworksSnapshot());
    deleteKeywordsModel.appendKeywordToDelete(keywordToDelete);
    deleteKeywordsModel.setCaseSensitive(false);
    deleteKeywordsModel.deleteKeywords();

    artworksListModel.foreachArtwork([&](int, Mocks::ArtworkMetadataMock *artwork) {
        auto *keywordsModel = artwork->getBasicModel();
        QCOMPARE(keywordsModel->getKeywordsCount(), 1);
        QVERIFY(!keywordsModel->containsKeyword(keywordToDelete));
    });
}
