#include "deletekeywords_tests.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/artworkmetadatamock.h"
#include "Mocks/coretestsenvironment.h"
#include "Mocks/artworksrepositorymock.h"
#include "../../xpiks-qt/Models/deletekeywordsviewmodel.h"
#include "../../xpiks-qt/Models/artworksrepository.h"
#include "../../xpiks-qt/Models/filteredartitemsproxymodel.h"

#define DECLARE_MODELS_AND_GENERATE(count) \
    Mocks::CoreTestsEnvironment environment; \
    Mocks::CommandManagerMock commandManagerMock; \
    Mocks::ArtItemsModelMock artItemsModelMock; \
    Mocks::ArtworksRepositoryMock artworksRepository(environment); \
    Models::FilteredArtItemsProxyModel filteredItemsModel; \
    Models::DeleteKeywordsViewModel deleteKeywordsModel; \
    commandManagerMock.InjectDependency(&artworksRepository); \
    commandManagerMock.InjectDependency(&artItemsModelMock); \
    commandManagerMock.InjectDependency(&deleteKeywordsModel); \
    filteredItemsModel.setSourceModel(&artItemsModelMock); \
    commandManagerMock.InjectDependency(&filteredItemsModel); \
    commandManagerMock.generateAndAddArtworks(count);

void DeleteKeywordsTests::smokeTest() {
    DECLARE_MODELS_AND_GENERATE(2);

    const QString keywordToDelete = "keyword1";
    QStringList keywords;
    keywords << keywordToDelete << "keyword2" << "keyword3";

    artItemsModelMock.foreachArtwork([&keywords](int, Mocks::ArtworkMetadataMock *artwork) {
        artwork->setKeywords(keywords);
    });

    filteredItemsModel.selectFilteredArtworks();
    filteredItemsModel.deleteKeywordsFromSelected();
    deleteKeywordsModel.appendKeywordToDelete(keywordToDelete);
    deleteKeywordsModel.deleteKeywords();

    artItemsModelMock.foreachArtwork([&](int, Mocks::ArtworkMetadataMock *artwork) {
        QCOMPARE(artwork->rowCount(), keywords.length() - 1);
        auto *keywordsModel = artwork->getBasicModel();
        QVERIFY(!keywordsModel->containsKeyword(keywordToDelete));
    });
}

void DeleteKeywordsTests::keywordsCombinedTest() {
    DECLARE_MODELS_AND_GENERATE(3);

    const QString keywordToDelete = "keywordToDelete";

    artItemsModelMock.foreachArtwork([&](int index, Mocks::ArtworkMetadataMock *artwork) {
        artwork->clearKeywords();
        artwork->appendKeyword("keyword" + QString::number(index));
        artwork->appendKeyword(keywordToDelete);
    });

    filteredItemsModel.selectFilteredArtworks();
    filteredItemsModel.deleteKeywordsFromSelected();

    QVERIFY(deleteKeywordsModel.containsCommonKeyword(keywordToDelete));

    deleteKeywordsModel.appendKeywordToDelete(keywordToDelete);
    deleteKeywordsModel.deleteKeywords();

    artItemsModelMock.foreachArtwork([&](int, Mocks::ArtworkMetadataMock *artwork) {
        auto *keywordsModel = artwork->getBasicModel();
        QVERIFY(!keywordsModel->containsKeyword(keywordToDelete));
    });
}

void DeleteKeywordsTests::doesNotDeleteOtherCaseTest() {
    DECLARE_MODELS_AND_GENERATE(3);

    const QString keywordToDelete = "keywordToDelete";

    artItemsModelMock.foreachArtwork([&](int index, Mocks::ArtworkMetadataMock *artwork) {
        artwork->clearKeywords();
        artwork->appendKeyword("keyword" + QString::number(index));
        artwork->appendKeyword(keywordToDelete);
    });

    filteredItemsModel.selectFilteredArtworks();
    filteredItemsModel.deleteKeywordsFromSelected();
    deleteKeywordsModel.appendKeywordToDelete(keywordToDelete.toUpper());
    deleteKeywordsModel.setCaseSensitive(true);
    deleteKeywordsModel.deleteKeywords();

    artItemsModelMock.foreachArtwork([&](int, Mocks::ArtworkMetadataMock *artwork) {
        auto *keywordsModel = artwork->getBasicModel();
        QCOMPARE(keywordsModel->getKeywordsCount(), 2);
        QVERIFY(keywordsModel->containsKeyword(keywordToDelete));
    });
}

void DeleteKeywordsTests::doesNotDeleteNoKeywordsTest() {
    DECLARE_MODELS_AND_GENERATE(3);

    const QString keywordToDelete = "keywordToDelete";

    artItemsModelMock.foreachArtwork([&](int index, Mocks::ArtworkMetadataMock *artwork) {
        artwork->clearKeywords();
        artwork->appendKeyword("keyword" + QString::number(index));
        artwork->appendKeyword(keywordToDelete);
    });

    filteredItemsModel.selectFilteredArtworks();
    filteredItemsModel.deleteKeywordsFromSelected();
    deleteKeywordsModel.deleteKeywords();

    artItemsModelMock.foreachArtwork([&](int, Mocks::ArtworkMetadataMock *artwork) {
        auto *keywordsModel = artwork->getBasicModel();
        QCOMPARE(keywordsModel->getKeywordsCount(), 2);
        QVERIFY(keywordsModel->containsKeyword(keywordToDelete));
    });
}

void DeleteKeywordsTests::deleteCaseInsensitiveTest() {
    DECLARE_MODELS_AND_GENERATE(5);

    const QString keywordToDelete = "keywordToDelete";

    artItemsModelMock.foreachArtwork([&](int index, Mocks::ArtworkMetadataMock *artwork) {
        artwork->clearKeywords();
        artwork->appendKeyword("keyword" + QString::number(index));
        artwork->appendKeyword(index % 2 == 0 ? keywordToDelete.toLower() : keywordToDelete.toUpper());
    });

    filteredItemsModel.selectFilteredArtworks();
    filteredItemsModel.deleteKeywordsFromSelected();
    deleteKeywordsModel.appendKeywordToDelete(keywordToDelete);
    deleteKeywordsModel.setCaseSensitive(false);
    deleteKeywordsModel.deleteKeywords();

    artItemsModelMock.foreachArtwork([&](int, Mocks::ArtworkMetadataMock *artwork) {
        auto *keywordsModel = artwork->getBasicModel();
        QCOMPARE(keywordsModel->getKeywordsCount(), 1);
        QVERIFY(!keywordsModel->containsKeyword(keywordToDelete));
    });
}
