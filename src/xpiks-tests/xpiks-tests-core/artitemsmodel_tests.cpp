#include "artitemsmodel_tests.h"
#include <QSignalSpy>
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/artworksrepositorymock.h"
#include "../../xpiks-qt/Models/filteredartitemsproxymodel.h"
#include "../../xpiks-qt/Models/artworksrepository.h"
#include "../../xpiks-qt/QuickBuffer/quickbuffer.h"
#include "../../xpiks-qt/KeywordsPresets/presetkeywordsmodel.h"
#include "../../xpiks-qt/Models/artworkproxymodel.h"
#include "Mocks/coretestsenvironment.h"

#define DECLARE_MODELS_AND_GENERATE(count, withVector) \
    Mocks::CoreTestsEnvironment environment; \
    Mocks::CommandManagerMock commandManagerMock;\
    Mocks::ArtworksListModelMock ArtworksListModelMock;\
    Mocks::ArtworksRepositoryMock artworksRepository(environment);\
    Models::FilteredArtItemsProxyModel filteredItemsModel;\
    commandManagerMock.InjectDependency(&artworksRepository);\
    commandManagerMock.InjectDependency(&ArtworksListModelMock);\
    filteredItemsModel.setSourceModel(&ArtworksListModelMock);\
    commandManagerMock.InjectDependency(&filteredItemsModel);\
    commandManagerMock.generateAndAddArtworks(count, withVector);

#define MODIFIED_TEST_START\
    QSignalSpy artItemsModifiedSpy(&ArtworksListModelMock, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

#define MODIFIED_TEST_END\
    QVERIFY(artItemsModifiedSpy.count() > 0);\
    QVERIFY(artItemsModifiedSpy.takeFirst().at(2).value<QVector<int> >().contains(Models::ArtItemsModel::IsModifiedRole));

void ArtItemsModelTests::removeUnavailableTest() {
    const int count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);

    for (int i = 0; i < count; ++i) {
        if (i%3 == 0) {
            ArtworksListModelMock.getArtwork(i)->setUnavailable();
        }
    }

    ArtworksListModelMock.removeUnavailableItems();

    for (int i = 0; i < ArtworksListModelMock.getArtworksCount(); ++i) {
        QVERIFY(!ArtworksListModelMock.getArtwork(i)->isUnavailable());
    }
}

void ArtItemsModelTests::unselectAllTest() {
    const int count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);

    for (int i = 0; i < count; ++i) {
        if (i%3 == 0) {
            ArtworksListModelMock.getArtwork(i)->setIsSelected(true);
        }
    }

    ArtworksListModelMock.forceUnselectAllItems();

    for (int i = 0; i < count; ++i) {
        QVERIFY(!ArtworksListModelMock.getArtwork(i)->isSelected());
    }
}

void ArtItemsModelTests::modificationChangesModifiedCountTest() {
    const int count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);

    const int index = 3;

    QCOMPARE(ArtworksListModelMock.getModifiedArtworksCount(), 0);
    ArtworksListModelMock.getArtwork(index)->setModified();
    QCOMPARE(ArtworksListModelMock.getModifiedArtworksCount(), 1);
    ArtworksListModelMock.getArtwork(index)->resetModified();
    QCOMPARE(ArtworksListModelMock.getModifiedArtworksCount(), 0);
}

void ArtItemsModelTests::removeArtworkDirectorySimpleTest() {
    const int count = 11;
    DECLARE_MODELS_AND_GENERATE(count, false);

    int indexToRemove = 1;

    int firstDirCount = artworksRepository.getFilesCountForDirectory(indexToRemove);

    QCOMPARE(ArtworksListModelMock.getArtworksCount(), count);
    ArtworksListModelMock.removeArtworksDirectory(indexToRemove);
    QCOMPARE(ArtworksListModelMock.getArtworksCount(), count - firstDirCount);
}

void ArtItemsModelTests::addRemoveOneByOneFewDirsTest() {
    // https://github.com/ribtoks/xpiks/issues/467
    const int count = 2;
    DECLARE_MODELS_AND_GENERATE(count, false);

    QCOMPARE(artworksRepository.getFilesCountForDirectory(0), 1);
    QCOMPARE(artworksRepository.getFilesCountForDirectory(1), 1);
    QCOMPARE(ArtworksListModelMock.getArtworksCount(), count);

    ArtworksListModelMock.removeArtworksDirectory(0);
    artworksRepository.cleanupEmptyDirectories();
    QCOMPARE(artworksRepository.rowCount(), 1);

    ArtworksListModelMock.removeArtworksDirectory(0);
    artworksRepository.cleanupEmptyDirectories();
    QCOMPARE(artworksRepository.rowCount(), 0);

    QCOMPARE(ArtworksListModelMock.getArtworksCount(), 0);
}

void ArtItemsModelTests::addRemoveOneByOneOneDirTest() {
    // https://github.com/ribtoks/xpiks/issues/467
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);

    QCOMPARE(artworksRepository.getFilesCountForDirectory(0), 1);
    QCOMPARE(ArtworksListModelMock.getArtworksCount(), count);

    ArtworksListModelMock.removeArtworksDirectory(0);
    artworksRepository.cleanupEmptyDirectories();
    QCOMPARE(artworksRepository.rowCount(), 0);
    QCOMPARE(ArtworksListModelMock.getArtworksCount(), 0);
}

void ArtItemsModelTests::setAllSavedResetsModifiedCountTest() {
    const int count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);
    QVector<int> selectedItems;

    for (int i = 0; i < count; ++i) {
        if (i%3 == 0) {
            ArtworksListModelMock.getArtwork(i)->setModified();
            selectedItems.append(i);
        }
    }

    QCOMPARE(ArtworksListModelMock.getModifiedArtworksCount(), selectedItems.count());

    ArtworksListModelMock.setSelectedItemsSaved(selectedItems);
    QCOMPARE(ArtworksListModelMock.getModifiedArtworksCount(), 0);
}

void ArtItemsModelTests::removingLockedArtworksTest() {
    const size_t count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);

    for (int i = 0; i < (int)count; ++i) {
        ArtworksListModelMock.getArtwork(i)->acquire();
    }

    QCOMPARE(ArtworksListModelMock.getFinalizationList().size(), (size_t)0);
    ArtworksListModelMock.deleteAllItems();
    QCOMPARE(ArtworksListModelMock.getFinalizationList().size(), count);
}

void ArtItemsModelTests::plainTextEditToEmptyKeywordsTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    ArtworksListModelMock.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");

    ArtworksListModelMock.plainTextEdit(0, "");
    QCOMPARE(ArtworksListModelMock.getMockArtwork(0)->getKeywords().length(), 0);
}

void ArtItemsModelTests::plainTextEditToOneKeywordTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    ArtworksListModelMock.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");

    QString keywords = "new keyword";
    QStringList result = QStringList() << keywords;

    ArtworksListModelMock.plainTextEdit(0, keywords);
    QCOMPARE(ArtworksListModelMock.getMockArtwork(0)->getKeywords(), result);
}

void ArtItemsModelTests::plainTextEditToSeveralKeywordsTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    ArtworksListModelMock.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");

    QString keywords = "new keyword, another one, new";
    QStringList result = QStringList() << "new keyword" << "another one" << "new";

    ArtworksListModelMock.plainTextEdit(0, keywords);
    QCOMPARE(ArtworksListModelMock.getMockArtwork(0)->getKeywords(), result);
}

void ArtItemsModelTests::plainTextEditToAlmostEmptyTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    ArtworksListModelMock.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");

    QString keywords = ",,, , , , , ,,,,   ";
    QStringList result = QStringList();

    ArtworksListModelMock.plainTextEdit(0, keywords);
    QCOMPARE(ArtworksListModelMock.getMockArtwork(0)->getKeywords(), result);
}

void ArtItemsModelTests::plainTextEditToMixedTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    ArtworksListModelMock.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");

    QString keywords = ",,, , ,word here , , ,,,,   ";
    QStringList result = QStringList() << "word here";

    ArtworksListModelMock.plainTextEdit(0, keywords);
    QCOMPARE(ArtworksListModelMock.getMockArtwork(0)->getKeywords(), result);
}

void ArtItemsModelTests::appendKeywordEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    MODIFIED_TEST_START;

    ArtworksListModelMock.appendKeyword(0, "brand new keyword");

    MODIFIED_TEST_END;
}

void ArtItemsModelTests::removeKeywordEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    ArtworksListModelMock.appendKeyword(0, "brand new keyword");
    ArtworksListModelMock.getMockArtwork(0)->resetModified();

    MODIFIED_TEST_START;

    ArtworksListModelMock.removeKeywordAt(0, 0);

    MODIFIED_TEST_END;
}

void ArtItemsModelTests::removeLastKeywordEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    ArtworksListModelMock.appendKeyword(0, "brand new keyword");
    ArtworksListModelMock.getMockArtwork(0)->resetModified();

    MODIFIED_TEST_START;

    ArtworksListModelMock.removeLastKeyword(0);

    MODIFIED_TEST_END;
}

void ArtItemsModelTests::plainTextEditEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    MODIFIED_TEST_START;

    ArtworksListModelMock.plainTextEdit(0, "brand,new,keyword");

    MODIFIED_TEST_END;
}

void ArtItemsModelTests::keywordEditEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    ArtworksListModelMock.appendKeyword(0, "keyword");
    ArtworksListModelMock.getMockArtwork(0)->resetModified();

    MODIFIED_TEST_START;

    ArtworksListModelMock.editKeyword(0, 0, "other");

    MODIFIED_TEST_END;
}

void ArtItemsModelTests::pasteKeywordsEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    MODIFIED_TEST_START;

    ArtworksListModelMock.pasteKeywords(0, QString("other,keywords,here").split(','));

    MODIFIED_TEST_END;
}

void ArtItemsModelTests::addSuggestedEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    MODIFIED_TEST_START;

    ArtworksListModelMock.addSuggestedKeywords(0, QString("suggested,keywords,here").split(','));

    MODIFIED_TEST_END;
}

void ArtItemsModelTests::fillFromQuickBufferEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);
    QuickBuffer::QuickBuffer quickBuffer;
    commandManagerMock.InjectDependency(&quickBuffer);
    quickBuffer.setDescription("description");
    quickBuffer.setTitle("title");
    quickBuffer.setKeywords(QStringList() << "keywords" << "here");

    MODIFIED_TEST_START;

    ArtworksListModelMock.fillFromQuickBuffer(0);

    MODIFIED_TEST_END;
}

void ArtItemsModelTests::addPresetEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    KeywordsPresets::PresetKeywordsModel presetKeywordsModel(environment);
    commandManagerMock.InjectDependency(&presetKeywordsModel);
    presetKeywordsModel.addItem("preset", QString("preset,keywords").split(','));

    MODIFIED_TEST_START;

    ArtworksListModelMock.addPreset(0, 0);

    MODIFIED_TEST_END;
}

void ArtItemsModelTests::proxyModelExitEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    Models::ArtworkProxyModel proxyModel;
    commandManagerMock.InjectDependency(&proxyModel);
    proxyModel.setSourceArtwork((QObject*)ArtworksListModelMock.getMockArtwork(0));
    proxyModel.setDescription("other description");

    ArtworksListModelMock.setUpdatesBlocked(false);

    MODIFIED_TEST_START;

    proxyModel.resetModel();

    MODIFIED_TEST_END;
}
