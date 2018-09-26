#include "combinedmodel_tests.h"
#include <QStringList>
#include <QVector>
#include <vector>
#include <QSignalSpy>
#include <Models/Editing/combinedartworksmodel.h>
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/commandmanagermock.h"
#include <Artworks/artworkelement.h>
#include <Artworks/basickeywordsmodel.h>
#include <Models/settingsmodel.h>
#include <UndoRedo/undoredomanager.h>
#include <KeywordsPresets/presetkeywordsmodel.h>

#define DECLARE_MODELS \
    Mocks::CoreTestsEnvironment environment;\
    KeywordsPresets::PresetKeywordsModel presetsManager(environment);\
    Models::CombinedArtworksModel combinedModel(presetsManager);

std::shared_ptr<Artworks::ArtworkMetadata> createArtworkMetadata(const QString &desc, const QString &title, const QStringList &keywords, int index=0) {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/random/file/path.jpg");
    artwork->initAsEmpty();
    artwork->appendKeywords(keywords);
    artwork->setTitle(title);
    artwork->setDescription(desc);
    artwork->resetModified();
    return artwork;
}

CombinedModelTests::CombinedModelTests(QObject *parent):
    QObject(parent)
{
}

void CombinedModelTests::trivialCombineNoItemsTest() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;

    combinedModel.resetModel();
    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), 0);
    QVERIFY(combinedModel.getDescription().isEmpty());
    QVERIFY(combinedModel.getTitle().isEmpty());
    QCOMPARE(combinedModel.getKeywordsCount(), 0);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
    QCOMPARE(combinedModel.getChangeDescription(), true);
    QCOMPARE(combinedModel.getChangeTitle(), true);
    QCOMPARE(combinedModel.getChangeKeywords(), true);
}

void CombinedModelTests::trivialCombineOneItemTest() {
    DECLARE_MODELS;

    QString desc = "Description for the item";
    QString title = "Item title";
    QStringList keywords;
    keywords << "keyword1" << "keyword2" << "keyword3";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(desc, title, keywords));

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), 1);
    QCOMPARE(combinedModel.getDescription(), desc);
    QCOMPARE(combinedModel.getTitle(), title);
    QCOMPARE(combinedModel.getKeywordsCount(), keywords.count());
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::combineSeveralSameItemsTest() {
    DECLARE_MODELS;

    QString desc = "Description for the item";
    QString title = "Item title";
    QStringList keywords;
    keywords << "keyword1" << "keyword2" << "keyword3";

    Artworks::ArtworksSnapshot snapshot;
    const int itemsToGenerate = 5;
    int numberOfItems = itemsToGenerate;
    while (numberOfItems--) {
        snapshot.append(createArtworkMetadata(desc, title, keywords));
    }

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), itemsToGenerate);
    QCOMPARE(combinedModel.getDescription(), desc);
    QCOMPARE(combinedModel.getTitle(), title);
    QCOMPARE(combinedModel.getKeywordsCount(), keywords.count());
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::combineSeveralWithEmptyFirstTest() {
    DECLARE_MODELS;

    QStringList keywords;
    keywords << "keyword1" << "keyword2" << "keyword3";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList(), 1));
    snapshot.append(createArtworkMetadata("Description1", "title1", keywords, 0));
    int size = (int)snapshot.size();

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), size);
    QVERIFY(combinedModel.getDescription().isEmpty());
    QVERIFY(combinedModel.getTitle().isEmpty());
    QCOMPARE(combinedModel.getKeywords(), keywords);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::combineSeveralWithEmptyManyTest() {
    DECLARE_MODELS;

    const QString commonKeyword = "Keyword1";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList(), 1));
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << commonKeyword << "keyword2", 0));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << commonKeyword, 2));

    int size = (int)snapshot.size();

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), size);
    QVERIFY(combinedModel.getDescription().isEmpty());
    QVERIFY(combinedModel.getTitle().isEmpty());
    QCOMPARE(combinedModel.getKeywordsCount(), 1);
    QCOMPARE(combinedModel.getKeywords()[0], commonKeyword);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::combineSeveralWithEmptyTest() {
    DECLARE_MODELS;

    const QString commonKeyword = "Keyword1";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << commonKeyword << "keyword2", 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList(), 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << commonKeyword, 2));

    int size = (int)snapshot.size();

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), size);
    QVERIFY(combinedModel.getDescription().isEmpty());
    QVERIFY(combinedModel.getTitle().isEmpty());
    QCOMPARE(combinedModel.getKeywordsCount(), 1);
    QCOMPARE(combinedModel.getKeywords()[0], commonKeyword);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::combineSeveralEmptyTest() {
    DECLARE_MODELS;

    const QString title = "title";
    const QString description = "description";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(description, title, QStringList(), 0));
    snapshot.append(createArtworkMetadata("", "", QStringList(), 1));
    snapshot.append(createArtworkMetadata("", "", QStringList(), 2));

    int size = (int)snapshot.size();

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), size);
    QCOMPARE(combinedModel.getTitle(), title);
    QCOMPARE(combinedModel.getDescription(), description);
    QCOMPARE(combinedModel.getKeywordsCount(), 0);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::combineAllDifferentItemsTest() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1", 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2", 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3", 2));

    int size = (int)snapshot.size();

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), size);
    QVERIFY(combinedModel.getDescription().isEmpty());
    QVERIFY(combinedModel.getTitle().isEmpty());
    QCOMPARE(combinedModel.getKeywordsCount(), 0);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::combineAllManyDifferentItemsTest() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1" << "keyword2", 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword3" << "keyword4" << "keyword5", 1));

    int size = (int)snapshot.size();

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), size);
    QVERIFY(combinedModel.getDescription().isEmpty());
    QVERIFY(combinedModel.getTitle().isEmpty());
    QCOMPARE(combinedModel.getKeywordsCount(), 0);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::combineAllManyLastDifferentItemsTest() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1" << "keyword2", 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword1" << "keyword2", 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3" << "keyword4", 1));

    int size = (int)snapshot.size();

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), size);
    QVERIFY(combinedModel.getDescription().isEmpty());
    QVERIFY(combinedModel.getTitle().isEmpty());
    QCOMPARE(combinedModel.getKeywordsCount(), 0);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::combineCommonInKeywordsTest() {
    DECLARE_MODELS;

    QString commonKeyword = "a common keyword";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1" << commonKeyword, 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2" << commonKeyword, 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3" << commonKeyword, 2));

    int size = (int)snapshot.size();
    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), size);
    QVERIFY(combinedModel.getDescription().isEmpty());
    QVERIFY(combinedModel.getTitle().isEmpty());
    QCOMPARE(combinedModel.getKeywordsCount(), 1);
    QCOMPARE(combinedModel.getKeywords()[0], commonKeyword);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::combineCommonInManyKeywordsTest() {
    DECLARE_MODELS;

    QString commonKeyword = "a common keyword";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1" << "keyword12" << commonKeyword, 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2" << "keyword22" << "keyword23" << commonKeyword, 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3" << commonKeyword, 2));

    int size = (int)snapshot.size();
    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), size);
    QVERIFY(combinedModel.getDescription().isEmpty());
    QVERIFY(combinedModel.getTitle().isEmpty());
    QCOMPARE(combinedModel.getKeywordsCount(), 1);
    QCOMPARE(combinedModel.getKeywords()[0], commonKeyword);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::combineCommonInTitleTest() {
    DECLARE_MODELS;

    QString commonTitle = "a common title";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", commonTitle, QStringList() << "Keyword1", 0));
    snapshot.append(createArtworkMetadata("Description2", commonTitle, QStringList() << "Keyword2", 1));
    snapshot.append(createArtworkMetadata("Description3", commonTitle, QStringList() << "Keyword3", 2));

    int size = (int)snapshot.size();
    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), size);
    QVERIFY(combinedModel.getDescription().isEmpty());
    QCOMPARE(combinedModel.getTitle(), commonTitle);
    QCOMPARE(combinedModel.getKeywordsCount(), 0);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::combineCommonInDescriptionTest() {
    DECLARE_MODELS;

    QString commonDescription = "a common Description1";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(commonDescription, "title1", QStringList() << "Keyword1", 0));
    snapshot.append(createArtworkMetadata(commonDescription, "title2", QStringList() << "Keyword2", 1));
    snapshot.append(createArtworkMetadata(commonDescription, "title3", QStringList() << "Keyword3", 2));

    int size = (int)snapshot.size();
    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getArtworksCount(), size);
    QVERIFY(combinedModel.getTitle().isEmpty());
    QCOMPARE(combinedModel.getDescription(), commonDescription);
    QCOMPARE(combinedModel.getKeywordsCount(), 0);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::editSeveralWithSameKeywordsTest() {
    DECLARE_MODELS;

    const QString commonDescription = "a common Description1";
    const QString commonTitle = "a common Title";
    const QStringList commonKeywords = QStringList() << "fox" << "dog" << "pastel" << "art";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(commonDescription, commonTitle, commonKeywords, 0));
    snapshot.append(createArtworkMetadata(commonDescription, commonTitle, commonKeywords, 1));
    snapshot.append(createArtworkMetadata(commonDescription, commonTitle, commonKeywords, 2));

    combinedModel.setArtworks(snapshot);
    combinedModel.pasteKeywords(QStringList() << "outdoors" << "tilt" << "pet");
    combinedModel.setChangeKeywords(true);
    combinedModel.getActionCommand(true)->execute();

    LOG_DEBUG << "Checking" << snapshot.size() << "items";

    for (auto &artwork: snapshot) {
        QStringList keywordsSlice = artwork->getKeywords().mid(0, 4);
        QCOMPARE(keywordsSlice, commonKeywords);
    }
}

void CombinedModelTests::recombineAfterRemoveDifferentTest() {
    DECLARE_MODELS;

    QString commonDescription = "a common Description1";
    QString commonKeyword = "keyword";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(commonDescription, "title1", QStringList() << "Keyword1" << commonKeyword, 0));
    snapshot.append(createArtworkMetadata(commonDescription, "title2", QStringList() << "Keyword2" << commonKeyword, 1));
    snapshot.append(createArtworkMetadata("Different description", "title3", QStringList() << "Keyword3", 2));

    int size = (int)snapshot.size();

    combinedModel.setArtworks(snapshot);

    combinedModel.setIsSelected(size - 1, true);
    combinedModel.removeSelectedArtworks();

    QCOMPARE(combinedModel.getArtworksCount(), size - 1);
    QVERIFY(combinedModel.getTitle().isEmpty());
    QCOMPARE(combinedModel.getDescription(), commonDescription);
    QCOMPARE(combinedModel.getKeywordsCount(), 1);
    QCOMPARE(combinedModel.getKeywords()[0], commonKeyword);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::recombineAfterRemoveAllButOneTest() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1" << "adfafdaf", 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2" << "21345425421", 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3" << "(*&^*&^*&&^%", 2));

    combinedModel.setArtworks(snapshot);

    combinedModel.setIsSelected(1, true);
    combinedModel.setIsSelected(2, true);
    combinedModel.removeSelectedArtworks();
    auto &first = combinedModel.accessItem(0)->getArtwork();

    QCOMPARE(combinedModel.getArtworksCount(), 1);
    QCOMPARE(combinedModel.getTitle(), first->getTitle());
    QCOMPARE(combinedModel.getDescription(), first->getDescription());
    QCOMPARE(combinedModel.getKeywordsCount(), first->getBasicModel().getKeywordsCount());
    QCOMPARE(combinedModel.getKeywords(), first->getKeywords());
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::recombineAfterChangesTest() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1" << "adfafdaf", 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2" << "21345425421", 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3" << "(*&^*&^*&&^%", 2));

    combinedModel.setArtworks(snapshot);

    combinedModel.appendKeyword("brand new keyword");
    combinedModel.setDescription(combinedModel.getDescription() + " new stuff here");
    combinedModel.setTitle(combinedModel.getTitle() + " new stuff here");

    combinedModel.setIsSelected(1, true);
    combinedModel.setIsSelected(2, true);
    combinedModel.removeSelectedArtworks();
    auto &first = combinedModel.accessItem(0)->getArtwork();

    QCOMPARE(combinedModel.getArtworksCount(), 1);
    QVERIFY(combinedModel.getTitle() != first->getTitle());
    QVERIFY(combinedModel.getDescription() != first->getDescription());
    QVERIFY(combinedModel.getKeywordsCount() != first->getBasicModel().getKeywordsCount());
    QVERIFY(combinedModel.getKeywords() != first->getKeywords());
    QCOMPARE(combinedModel.areKeywordsModified(), true);
}

void CombinedModelTests::isNotModifiedAfterTitleDescEditTest() {
    DECLARE_MODELS;

    QString commonDescription = "a common Description1";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(commonDescription, "title1", QStringList() << "Keyword1", 0));
    snapshot.append(createArtworkMetadata(commonDescription, "title2", QStringList() << "Keyword2", 1));
    snapshot.append(createArtworkMetadata(commonDescription, "title3", QStringList() << "Keyword3", 2));

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.areKeywordsModified(), false);

    combinedModel.setDescription("Brand new description");
    QCOMPARE(combinedModel.areKeywordsModified(), false);

    combinedModel.setTitle("Brand new title");
    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::isModifiedAfterKeywordsAppendTest() {
    DECLARE_MODELS;

    QString commonDescription = "a common Description1";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(commonDescription, "title1", QStringList() << "Keyword1", 0));
    snapshot.append(createArtworkMetadata(commonDescription, "title2", QStringList() << "Keyword2", 1));
    snapshot.append(createArtworkMetadata(commonDescription, "title3", QStringList() << "Keyword3", 2));

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.areKeywordsModified(), false);

    combinedModel.appendKeyword("Brand new keyword");

    QCOMPARE(combinedModel.areKeywordsModified(), true);
}

void CombinedModelTests::isModifiedAfterKeywordRemovalTest() {
    DECLARE_MODELS;

    QString commonKeyword = "a common keyword";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1" << commonKeyword, 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2" << commonKeyword, 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3" << commonKeyword, 2));

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.areKeywordsModified(), false);

    combinedModel.removeLastKeyword();

    QCOMPARE(combinedModel.areKeywordsModified(), true);
}

void CombinedModelTests::isModifiedAfterKeywordEditTest() {
    DECLARE_MODELS;

    QString commonKeyword = "a common keyword";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1" << commonKeyword, 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2" << commonKeyword, 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3" << commonKeyword, 2));

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.areKeywordsModified(), false);

    combinedModel.editKeyword(0, "another keyword");

    QCOMPARE(combinedModel.areKeywordsModified(), true);
}

void CombinedModelTests::isModifiedAfterKeywordsClearTest() {
    DECLARE_MODELS;

    QString commonKeyword = "a common keyword";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1" << commonKeyword, 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2" << commonKeyword, 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3" << commonKeyword, 2));

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.areKeywordsModified(), false);

    combinedModel.clearKeywords();

    QCOMPARE(combinedModel.areKeywordsModified(), true);
}

void CombinedModelTests::isNotModifiedAfterEmptyKeywordsClearTest() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1", 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2", 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3", 2));

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.areKeywordsModified(), false);

    combinedModel.clearKeywords();

    QCOMPARE(combinedModel.areKeywordsModified(), false);
}

void CombinedModelTests::isModifiedStatusNotResetWithOtherTest() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1", 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2", 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3", 2));

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.areKeywordsModified(), false);
    combinedModel.appendKeyword("new keyword");
    combinedModel.removeLastKeyword();
    QCOMPARE(combinedModel.areKeywordsModified(), true);

    combinedModel.clearKeywords();

    QCOMPARE(combinedModel.areKeywordsModified(), true);
}

void CombinedModelTests::initArtworksEmitsRowsInsertTest() {
    DECLARE_MODELS;

    QString commonKeyword = "a common keyword";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1" << commonKeyword, 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2" << commonKeyword, 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3" << commonKeyword, 2));

    QSignalSpy resetSpy(&combinedModel, SIGNAL(modelReset()));

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.areKeywordsModified(), false);
    QCOMPARE(resetSpy.count(), 1);
}

void CombinedModelTests::initEmptyArtworksDoesNotEmitTest() {
    DECLARE_MODELS;

    QSignalSpy addSpy(&combinedModel, SIGNAL(rowsInserted(QModelIndex,int,int)));

    Artworks::ArtworksSnapshot snapshot;
    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.areKeywordsModified(), false);
    QCOMPARE(addSpy.count(), 0);
    QCOMPARE(combinedModel.getChangeDescription(), false);
    QCOMPARE(combinedModel.getChangeTitle(), false);
    QCOMPARE(combinedModel.getChangeKeywords(), false);
}

void CombinedModelTests::initOneArtworkEnablesAllFields() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1", 0));

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.areKeywordsModified(), false);
    QCOMPARE(combinedModel.getChangeDescription(), true);
    QCOMPARE(combinedModel.getChangeTitle(), true);
    QCOMPARE(combinedModel.getChangeKeywords(), true);
}

void CombinedModelTests::initManyArtworksDoesNotEnableAllFields() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1", 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2", 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3", 2));

    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.areKeywordsModified(), false);
    QCOMPARE(combinedModel.getChangeDescription(), false);
    QCOMPARE(combinedModel.getChangeTitle(), false);
    QCOMPARE(combinedModel.getChangeKeywords(), false);
}

void CombinedModelTests::resetModelClearsEverythingTest() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1", 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "Keyword2", 1));
    snapshot.append(createArtworkMetadata("Description3", "title3", QStringList() << "Keyword3", 2));

    combinedModel.setArtworks(snapshot);

    combinedModel.resetModel();

    QCOMPARE(combinedModel.getArtworksCount(), 0);
    QVERIFY(combinedModel.getDescription().isEmpty());
    QVERIFY(combinedModel.getTitle().isEmpty());
    QCOMPARE(combinedModel.getKeywordsCount(), 0);
    QCOMPARE(combinedModel.areKeywordsModified(), false);
    // TEMPORARY
    QCOMPARE(combinedModel.getChangeDescription(), true);
    QCOMPARE(combinedModel.getChangeTitle(), true);
    QCOMPARE(combinedModel.getChangeKeywords(), true);
}

void CombinedModelTests::appendNewKeywordEmitsCountChangedTest() {
    DECLARE_MODELS;

    QSignalSpy addSpy(&combinedModel, SIGNAL(keywordsCountChanged()));

    combinedModel.appendKeyword("new keyword");

    QCOMPARE(addSpy.count(), 1);
}

void CombinedModelTests::appendExistingKeywordDoesNotEmitTest() {
    DECLARE_MODELS;

    QString keyword = "new keyword";

    combinedModel.appendKeyword(keyword);

    QSignalSpy addSpy(&combinedModel, SIGNAL(keywordsCountChanged()));

    combinedModel.appendKeyword(keyword);

    QCOMPARE(addSpy.count(), 0);
}

void CombinedModelTests::pasteNewKeywordsEmitsCountChangedTest() {
    DECLARE_MODELS;

    QSignalSpy addSpy(&combinedModel, SIGNAL(keywordsCountChanged()));

    combinedModel.pasteKeywords(QStringList() << "new keyword" << "another keyword");

    QCOMPARE(addSpy.count(), 1);
}

void CombinedModelTests::pasteExistingKeywordsDoesNotEmitTest() {
    DECLARE_MODELS;

    QStringList keywords;
    keywords << "new keyword" << "another keyword";

    combinedModel.pasteKeywords(keywords);

    QSignalSpy addSpy(&combinedModel, SIGNAL(keywordsCountChanged()));
    combinedModel.pasteKeywords(keywords);

    QCOMPARE(addSpy.count(), 0);
}

void CombinedModelTests::editKeywordDoesNotEmitCountChangedTest() {
    DECLARE_MODELS;

    QString keyword = "new keyword";

    combinedModel.appendKeyword(keyword);

    QSignalSpy addSpy(&combinedModel, SIGNAL(keywordsCountChanged()));

    combinedModel.editKeyword(0, "another");

    QCOMPARE(addSpy.count(), 0);
}

void CombinedModelTests::notSavedAfterAllDisabledTest() {
    DECLARE_MODELS;

    QString commonDescription = "a common Description1";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(commonDescription, "title1", QStringList() << "Keyword1" << "keyword2", 0));
    snapshot.append(createArtworkMetadata(commonDescription, "title2", QStringList() << "Keyword2", 0));

    combinedModel.resetModel();
    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.areKeywordsModified(), false);

    combinedModel.setDescription("Brand new description");
    combinedModel.setTitle("Brand new title");
    combinedModel.appendKeyword("brand new keyword");

    combinedModel.setChangeDescription(false);
    combinedModel.setChangeTitle(false);
    combinedModel.setChangeKeywords(false);

    combinedModel.getActionCommand(true)->execute();

    for (auto &item: snapshot) { QCOMPARE(item->isModified(), false); }
}

void CombinedModelTests::notSavedAfterNothingModifiedTest() {
    DECLARE_MODELS;

    QString commonDescription = "a common Description1";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(commonDescription, "title1", QStringList() << "Keyword1", 0));

    combinedModel.resetModel();
    combinedModel.setArtworks(snapshot);

    combinedModel.getActionCommand(true)->execute();

    for (auto &item: snapshot) {
        QCOMPARE(item->isModified(), false);
    }
}

void CombinedModelTests::notSavedAfterModifiedDisabledTest() {
    DECLARE_MODELS;

    QString commonDescription = "a common Description1";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(commonDescription, "title1", QStringList() << "Keyword1", 0));

    combinedModel.resetModel();
    combinedModel.setArtworks(snapshot);

    combinedModel.setDescription("Brand new description");
    combinedModel.setChangeDescription(false);
    combinedModel.getActionCommand(true)->execute();
    for (auto &item: snapshot) { QCOMPARE(item->isModified(), false); }

    combinedModel.setTitle("Brand new title");
    combinedModel.setChangeTitle(false);
    combinedModel.getActionCommand(true)->execute();
    for (auto &item: snapshot) { QCOMPARE(item->isModified(), false); }
}

void CombinedModelTests::savedAfterModifiedDescriptionTest() {
    DECLARE_MODELS;

    QString commonDescription = "a common Description1";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(commonDescription, "title1", QStringList() << "Keyword1", 0));

    combinedModel.resetModel();
    combinedModel.setArtworks(snapshot);

    combinedModel.setDescription("Brand new description");
    combinedModel.getActionCommand(true)->execute();
    for (auto &item: snapshot) { QCOMPARE(item->isModified(), true); }
}

void CombinedModelTests::savedAfterModifiedTitleTest() {
    DECLARE_MODELS;

    QString commonDescription = "a common Description1";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(commonDescription, "title1", QStringList() << "Keyword1", 0));

    combinedModel.resetModel();
    combinedModel.setArtworks(snapshot);

    combinedModel.setTitle("Brand new title");
    combinedModel.getActionCommand(true)->execute();
    for (auto &item: snapshot) { QCOMPARE(item->isModified(), true); }
}

void CombinedModelTests::savedAfterKeywordsModifiedTest() {
    DECLARE_MODELS;

    QString commonDescription = "a common Description1";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(commonDescription, "title1", QStringList() << "Keyword1", 0));

    combinedModel.resetModel();
    combinedModel.setArtworks(snapshot);

    combinedModel.appendKeyword("Brand new keyword");
    combinedModel.getActionCommand(true)->execute();
    for (auto &item: snapshot) { QCOMPARE(item->isModified(), true); }
}

void CombinedModelTests::savedIfMoreThanOneButNotModifiedTest() {
    DECLARE_MODELS;

    QString commonDescription = "a common Description1";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata(commonDescription, "title1", QStringList() << "Keyword1", 0));
    snapshot.append(createArtworkMetadata(commonDescription, "title2", QStringList() << "Keyword2", 1));
    snapshot.append(createArtworkMetadata(commonDescription, "title3", QStringList() << "Keyword3", 2));

    combinedModel.resetModel();
    combinedModel.setArtworks(snapshot);

    combinedModel.getActionCommand(true)->execute();

    for (auto &item: snapshot) { QCOMPARE(item->isModified(), true); }
}

void CombinedModelTests::caseIsPreservedForOneItemTest() {
    DECLARE_MODELS;

    QStringList keywords = QStringList() << "Keyword1" << "keyworD2";

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", keywords, 0));

    combinedModel.resetModel();
    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getKeywords(), keywords);
}

void CombinedModelTests::caseIsPreservedForSeveralItemsTest() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList() << "Keyword1" << "keyworD2", 0));
    snapshot.append(createArtworkMetadata("Description2", "title2", QStringList() << "keyworD1" << "Keyword2", 1));

    combinedModel.resetModel();
    combinedModel.setArtworks(snapshot);

    QCOMPARE(combinedModel.getKeywordsCount(), 0);
}

void CombinedModelTests::clearKeywordsFiresKeywordsCountTest() {
    DECLARE_MODELS;

    Artworks::ArtworksSnapshot snapshot;
    snapshot.append(createArtworkMetadata("Description1", "title1", QStringList(), 0));

    combinedModel.resetModel();
    combinedModel.setArtworks(snapshot);

    QSignalSpy keywordsCountChangedSpy(&combinedModel, SIGNAL(keywordsCountChanged()));
    combinedModel.clearKeywords();

    QCOMPARE(keywordsCountChangedSpy.count(), 0);

    combinedModel.appendKeyword("test");
    keywordsCountChangedSpy.clear();
    combinedModel.clearKeywords();
    QCOMPARE(keywordsCountChangedSpy.count(), 1);
}
