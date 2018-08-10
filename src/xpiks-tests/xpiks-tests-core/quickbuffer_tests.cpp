#include "quickbuffer_tests.h"
#include <QSignalSpy>
#include "Mocks/commandmanagermock.h"
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/artworkmetadatamock.h"
#include "Mocks/coretestsenvironment.h"
#include "Mocks/artworksrepositorymock.h"
#include "Mocks/artworksupdatermock.h"
#include "Mocks/microstockclientsmock.h"
#include <Models/Session/recentdirectoriesmodel.h>
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Models/Editing/quickbuffer.h>
#include <Models/Editing/artworkproxymodel.h>
#include <Models/Editing/combinedartworksmodel.h>
#include <Models/settingsmodel.h>
#include <Models/switchermodel.h>
#include <Models/Editing/currenteditablemodel.h>
#include <Models/Connectivity/proxysettings.h>
#include <KeywordsPresets/presetkeywordsmodel.h>
#include <UndoRedo/undoredomanager.h>
#include <Connectivity/requestsservice.h>
#include <Suggestion/keywordssuggestor.h>
#include <Suggestion/suggestionartwork.h>

#define DECLARE_MODELS_AND_GENERATE(count) \
    Mocks::CoreTestsEnvironment environment; \
    UndoRedo::UndoRedoManager undoRedoManager;\
    Mocks::CommandManagerMock commandManager(undoRedoManager);\
    Models::RecentDirectoriesModel recentDirectories(environment);\
    recentDirectories.initialize();\
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories);\
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository);\
    KeywordsPresets::PresetKeywordsModel keywordsPresets(environment);\
    Models::SettingsModel settingsModel(environment);\
    settingsModel.initializeConfigs();\
    Models::FilteredArtworksListModel filteredArtworksModel(\
    artworksListModel, commandManager, keywordsPresets, settingsModel);\
    Models::CurrentEditableModel currentEditableModel;\
    Models::QuickBuffer quickBuffer(currentEditableModel, commandManager);\
    artworksListModel.generateAndAddArtworks(count);\
    Common::connectTarget<Models::QuickBufferMessage>(\
    quickBuffer, { filteredArtworksModel });\
    Common::connectTarget<std::shared_ptr<Models::ICurrentEditable>>(\
    currentEditableModel, { artworksListModel });

void QuickBufferTests::copyArtworkToQuickBufferTest() {
    DECLARE_MODELS_AND_GENERATE(1);
    QVERIFY(quickBuffer.getIsEmpty());

    const QString titleForQB = "title for quick buffer";
    const QString descriptionForQB = "desc for quick buffer";
    QStringList keywordsForQB;
    keywordsForQB << "brand" << "new" << "keywords";

    auto *artwork = artworksListModel.getMockArtwork(0);
    artwork->set(titleForQB, descriptionForQB, keywordsForQB);
    filteredArtworksModel.copyToQuickBuffer(0);

    QCOMPARE(quickBuffer.getTitle(), titleForQB);
    QCOMPARE(quickBuffer.getDescription(), descriptionForQB);
    QCOMPARE(quickBuffer.getKeywords(), keywordsForQB);
}

void QuickBufferTests::copyProxyModelToQuickBufferTest() {
    DECLARE_MODELS_AND_GENERATE(1);
    QVERIFY(quickBuffer.getIsEmpty());
    Mocks::ArtworksUpdaterMock updater;
    Models::ArtworkProxyModel proxyModel(commandManager, keywordsPresets, updater);
    Common::connectTarget<std::shared_ptr<Models::ICurrentEditable>>(currentEditableModel, { proxyModel });

    const QString titleForQB = "title for quick buffer";
    const QString descriptionForQB = "desc for quick buffer";
    QStringList keywordsForQB;
    keywordsForQB << "brand" << "new" << "keywords";

    proxyModel.setSourceArtwork(artworksListModel.getArtwork(0));
    proxyModel.setTitle(titleForQB);
    proxyModel.setDescription(descriptionForQB);
    proxyModel.setKeywords(keywordsForQB);

    quickBuffer.copyFromCurrentEditable();

    QCOMPARE(quickBuffer.getTitle(), titleForQB);
    QCOMPARE(quickBuffer.getDescription(), descriptionForQB);
    QCOMPARE(quickBuffer.getKeywords(), keywordsForQB);
}

void QuickBufferTests::copyCombinedModelToQuickBufferTest() {
    DECLARE_MODELS_AND_GENERATE(1);
    QVERIFY(quickBuffer.getIsEmpty());
    Models::CombinedArtworksModel combinedModel(commandManager, keywordsPresets);
    Common::connectSource<Models::QuickBufferMessage>(combinedModel, {quickBuffer});

    const QString titleForQB = "title for quick buffer";
    const QString descriptionForQB = "desc for quick buffer";
    QStringList keywordsForQB;
    keywordsForQB << "brand" << "new" << "keywords";

    combinedModel.setTitle(titleForQB);
    combinedModel.setDescription(descriptionForQB);
    combinedModel.setKeywords(keywordsForQB);

    combinedModel.copyToQuickBuffer();

    QCOMPARE(quickBuffer.getTitle(), titleForQB);
    QCOMPARE(quickBuffer.getDescription(), descriptionForQB);
    QCOMPARE(quickBuffer.getKeywords(), keywordsForQB);
}

void QuickBufferTests::copyHalfEmptyArtworkToQuickBufferTest() {
    DECLARE_MODELS_AND_GENERATE(1);    
    QVERIFY(quickBuffer.getIsEmpty());

    const QString previousDescription = "Previous description int QB";
    quickBuffer.setDescription(previousDescription);

    const QString titleForQB = "title for quick buffer";
    const QString descriptionForQB = "";
    QStringList keywordsForQB;
    keywordsForQB << "brand" << "new" << "keywords";

    auto *artwork = artworksListModel.getMockArtwork(0);
    artwork->set(titleForQB, descriptionForQB, keywordsForQB);
    filteredArtworksModel.copyToQuickBuffer(0);

    QCOMPARE(quickBuffer.getTitle(), titleForQB);
    QCOMPARE(quickBuffer.getDescription(), previousDescription);
    QCOMPARE(quickBuffer.getKeywords(), keywordsForQB);
}

void QuickBufferTests::copyHalfEmptyProxyModelToQuickBufferTest() {
    DECLARE_MODELS_AND_GENERATE(1);
    QVERIFY(quickBuffer.getIsEmpty());
    Mocks::ArtworksUpdaterMock updater;
    Models::ArtworkProxyModel proxyModel(commandManager, keywordsPresets, updater);
    Common::connectTarget<std::shared_ptr<Models::ICurrentEditable>>(currentEditableModel, { proxyModel });

    QStringList prevKeywordsForQB;
    prevKeywordsForQB << "quite" << "old" << "keywords";
    quickBuffer.setKeywords(prevKeywordsForQB);

    const QString titleForQB = "title for quick buffer";
    const QString descriptionForQB = "desc for quick buffer";
    QStringList keywordsForQB;

    proxyModel.setSourceArtwork(artworksListModel.getArtwork(0));
    proxyModel.setTitle(titleForQB);
    proxyModel.setDescription(descriptionForQB);
    proxyModel.setKeywords(keywordsForQB);

    quickBuffer.copyFromCurrentEditable();

    QCOMPARE(quickBuffer.getTitle(), titleForQB);
    QCOMPARE(quickBuffer.getDescription(), descriptionForQB);
    QCOMPARE(quickBuffer.getKeywords(), prevKeywordsForQB);
}

void QuickBufferTests::copyHalfEmptyCombinedModelToQuickBufferTest() {
    DECLARE_MODELS_AND_GENERATE(1);
    QVERIFY(quickBuffer.getIsEmpty());
    Models::CombinedArtworksModel combinedModel(commandManager, keywordsPresets);
    Common::connectTarget<Models::QuickBufferMessage>(quickBuffer, {combinedModel});

    QStringList prevKeywordsForQB;
    prevKeywordsForQB << "quite" << "old" << "keywords";
    quickBuffer.setKeywords(prevKeywordsForQB);

    const QString previousDescription = "Previous description int QB";
    quickBuffer.setDescription(previousDescription);

    const QString titleForQB = "title for quick buffer";
    const QString descriptionForQB = "";
    QStringList keywordsForQB;

    combinedModel.setTitle(titleForQB);
    combinedModel.setDescription(descriptionForQB);
    combinedModel.setKeywords(keywordsForQB);

    combinedModel.copyToQuickBuffer();

    QCOMPARE(quickBuffer.getTitle(), titleForQB);
    QCOMPARE(quickBuffer.getDescription(), previousDescription);
    QCOMPARE(quickBuffer.getKeywords(), prevKeywordsForQB);
}

void QuickBufferTests::copyKeywordsSuggestorToQuickBufferTest() {
    DECLARE_MODELS_AND_GENERATE(2);
    Models::SwitcherModel switcher(environment);
    switcher.initialize();
    Suggestion::KeywordsSuggestor suggestor(switcher,
                                            settingsModel,
                                            environment);
    suggestor.initialize();
    Common::connectTarget<Models::QuickBufferMessage>(quickBuffer, {suggestor});

    quickBuffer.setTitle("own title");
    quickBuffer.setDescription("own description");
    quickBuffer.setKeywords(QStringList() << "some" << "other" << "keywords");

    const QString title = "title";
    const QString description = "description";
    const QStringList keywords = QStringList() << "a keyword1" << "b keyword2";

    std::vector<std::shared_ptr<Suggestion::SuggestionArtwork>> fakeSuggestions = {
        std::make_shared<Suggestion::SuggestionArtwork>("url", title, description, keywords)
    };
    suggestor.setFakeSuggestions(fakeSuggestions);

    suggestor.copyToQuickBuffer(0);

    QCOMPARE(quickBuffer.getTitle(), title);
    QCOMPARE(quickBuffer.getDescription(), description);
    QStringList qbKeywords = quickBuffer.getKeywords();
    qbKeywords.sort();
    QCOMPARE(qbKeywords, keywords);
}

void QuickBufferTests::applyQuickBufferToArtworkTest() {
    DECLARE_MODELS_AND_GENERATE(2);

    const QString titleForQB = "title for quick buffer";
    const QString descriptionForQB = "desc for quick buffer";
    QStringList keywordsForQB;
    keywordsForQB << "brand" << "new" << "keywords";

    quickBuffer.setTitle(titleForQB);
    quickBuffer.setDescription(descriptionForQB);
    for (auto &keyword: keywordsForQB) {
        quickBuffer.appendKeyword(keyword);
    }

    filteredArtworksModel.registerCurrentItem(1);

    bool success = quickBuffer.copyToCurrentEditable();
    QVERIFY(success);

    auto *artwork = artworksListModel.getMockArtwork(1);
    QCOMPARE(artwork->getTitle(), titleForQB);
    QCOMPARE(artwork->getDescription(), descriptionForQB);
    QCOMPARE(artwork->getKeywords(), keywordsForQB);
}

void QuickBufferTests::applyQuickBufferToProxyModelTest() {
    DECLARE_MODELS_AND_GENERATE(2);
    Mocks::ArtworksUpdaterMock updater;
    Models::ArtworkProxyModel proxyModel(commandManager, keywordsPresets, updater);
    Common::connectTarget<std::shared_ptr<Models::ICurrentEditable>>(currentEditableModel, { proxyModel });

    const QString titleForQB = "title for quick buffer";
    const QString descriptionForQB = "desc for quick buffer";
    QStringList keywordsForQB;
    keywordsForQB << "brand" << "new" << "keywords";

    quickBuffer.setTitle(titleForQB);
    quickBuffer.setDescription(descriptionForQB);
    quickBuffer.setKeywords(keywordsForQB);

    proxyModel.setSourceArtwork(artworksListModel.getArtwork(1));

    bool success = quickBuffer.copyToCurrentEditable();
    QVERIFY(success);

    QCOMPARE(proxyModel.getTitle(), titleForQB);
    QCOMPARE(proxyModel.getDescription(), descriptionForQB);
    QCOMPARE(proxyModel.getKeywords(), keywordsForQB);

    auto *artwork = artworksListModel.getMockArtwork(1);
    QCOMPARE(artwork->getTitle(), titleForQB);
    QCOMPARE(artwork->getDescription(), descriptionForQB);
    QCOMPARE(artwork->getKeywords(), keywordsForQB);
}

void QuickBufferTests::applyQuickBufferToCombinedModelTest() {
    DECLARE_MODELS_AND_GENERATE(2);

    const QString titleForQB = "title for quick buffer";
    const QString descriptionForQB = "desc for quick buffer";
    QStringList keywordsForQB;
    keywordsForQB << "brand" << "new" << "keywords";

    quickBuffer.setTitle(titleForQB);
    quickBuffer.setDescription(descriptionForQB);
    quickBuffer.pasteKeywords(keywordsForQB);

    Models::CombinedArtworksModel combinedModel(commandManager, keywordsPresets);
    Common::connectTarget<std::shared_ptr<Models::ICurrentEditable>>(currentEditableModel, { combinedModel });
    combinedModel.setArtworks(artworksListModel.createArtworksSnapshot());

    bool success = quickBuffer.copyToCurrentEditable();
    QVERIFY(success);

    QCOMPARE(combinedModel.getTitle(), titleForQB);
    QCOMPARE(combinedModel.getDescription(), descriptionForQB);
    QCOMPARE(combinedModel.getKeywords(), keywordsForQB);
}

void QuickBufferTests::applyHalfEmptyQuickBufferToArtworkTest() {
    DECLARE_MODELS_AND_GENERATE(2);

    const QString titleForQB = "title for quick buffer";
    const QString descriptionForQB = "";
    QStringList keywordsForArtwork;
    keywordsForArtwork << "brand" << "new" << "keywords";

    auto *artwork = artworksListModel.getMockArtwork(1);
    const QString prevDescription = "some description";
    artwork->setDescription(prevDescription);
    artwork->setKeywords(keywordsForArtwork);

    quickBuffer.setTitle(titleForQB);
    quickBuffer.setDescription(descriptionForQB);
    // quickBuffer.setKeywords(keywordsForQB);

    filteredArtworksModel.registerCurrentItem(1);

    bool success = quickBuffer.copyToCurrentEditable();
    QVERIFY(success);
    QCOMPARE(artwork->getTitle(), titleForQB);
    QCOMPARE(artwork->getDescription(), prevDescription);
    QCOMPARE(artwork->getKeywords(), keywordsForArtwork);
}

void QuickBufferTests::applyHalfEmptyQuickBufferToProxyModelTest() {
    DECLARE_MODELS_AND_GENERATE(2);
    Mocks::ArtworksUpdaterMock updater;
    Models::ArtworkProxyModel proxyModel(commandManager, keywordsPresets, updater);
    Common::connectTarget<std::shared_ptr<Models::ICurrentEditable>>(currentEditableModel, { proxyModel });

    const QString titleForModel = "title for quick buffer";
    const QString descriptionForQB = "desc for quick buffer";
    QStringList keywordsForQB;
    keywordsForQB << "brand" << "new" << "keywords";

    //quickBuffer.setTitle(titleForQB);
    quickBuffer.setDescription(descriptionForQB);
    quickBuffer.setKeywords(keywordsForQB);

    proxyModel.setSourceArtwork(artworksListModel.getArtwork(1));
    proxyModel.setTitle(titleForModel);

    bool success = quickBuffer.copyToCurrentEditable();
    QVERIFY(success);

    QCOMPARE(proxyModel.getTitle(), titleForModel);
    QCOMPARE(proxyModel.getDescription(), descriptionForQB);
    QCOMPARE(proxyModel.getKeywords(), keywordsForQB);

    auto *artwork = artworksListModel.getMockArtwork(1);
    QCOMPARE(artwork->getTitle(), titleForModel);
    QCOMPARE(artwork->getDescription(), descriptionForQB);
    QCOMPARE(artwork->getKeywords(), keywordsForQB);
}

void QuickBufferTests::applyHalfEmptyQuickBufferToCombinedModelTest() {
    DECLARE_MODELS_AND_GENERATE(2);
    Models::CombinedArtworksModel combinedModel(commandManager, keywordsPresets);
    Common::connectTarget<std::shared_ptr<Models::ICurrentEditable>>(currentEditableModel, { combinedModel });

    const QString titleForQB = "title for quick buffer";
    const QString descriptionForModel = "desc for quick buffer";
    QStringList keywordsForModel;
    keywordsForModel << "brand" << "new" << "keywords";

    quickBuffer.setTitle(titleForQB);
    //quickBuffer.setDescription(descriptionForModel);
    //quickBuffer.setKeywords(keywordsForModel);

    combinedModel.setArtworks(artworksListModel.createArtworksSnapshot());

    combinedModel.setDescription(descriptionForModel);
    combinedModel.setKeywords(keywordsForModel);

    bool success = quickBuffer.copyToCurrentEditable();
    QVERIFY(success);

    QCOMPARE(combinedModel.getTitle(), titleForQB);
    QCOMPARE(combinedModel.getDescription(), descriptionForModel);
    QCOMPARE(combinedModel.getKeywords(), keywordsForModel);
}

void QuickBufferTests::cannotApplyWhenNoCurrentItemTest() {
    DECLARE_MODELS_AND_GENERATE(2);

    const QString titleForQB = "title for quick buffer";
    const QString descriptionForQB = "desc for quick buffer";
    QStringList keywordsForQB;
    keywordsForQB << "brand" << "new" << "keywords";

    auto *artwork = artworksListModel.getMockArtwork(1);
    const QString title = artwork->getTitle();
    const QString description = artwork->getDescription();
    QStringList keywords = artwork->getKeywords();

    quickBuffer.setTitle(titleForQB);
    quickBuffer.setDescription(descriptionForQB);
    quickBuffer.setKeywords(keywordsForQB);

    filteredArtworksModel.registerCurrentItem(1);
    currentEditableModel.clearCurrentItem();

    bool success = quickBuffer.copyToCurrentEditable();
    QVERIFY(!success);

    QCOMPARE(artwork->getTitle(), title);
    QCOMPARE(artwork->getDescription(), description);
    QCOMPARE(artwork->getKeywords(), keywords);
}

void QuickBufferTests::appendRemoveKeywordsTest() {
    DECLARE_MODELS_AND_GENERATE(1);
    QStringList keywordsForQB;
    keywordsForQB << "brand" << "new" << "keywords";

    for (auto &keyword: keywordsForQB) {
        quickBuffer.appendKeyword(keyword);
    }

    QCOMPARE(quickBuffer.getKeywords(), keywordsForQB);

    quickBuffer.removeKeywordAt(0);
    quickBuffer.removeKeywordAt(1);
    quickBuffer.removeLastKeyword();

    QVERIFY(quickBuffer.getKeywords().isEmpty());
}
