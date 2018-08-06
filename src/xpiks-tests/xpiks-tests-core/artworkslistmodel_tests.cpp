#include "artworkslistmodel_tests.h"
#include <QSignalSpy>
#include "Mocks/coretestsenvironment.h"
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/artworksrepositorymock.h"
#include <Models/Artworks/artworksrepository.h>
#include <Models/Session/recentdirectoriesmodel.h>
#include <Models/Editing/quickbuffer.h>
#include <Models/Editing/currenteditablemodel.h>
#include <Models/Editing/artworkproxymodel.h>
#include <UndoRedo/undoredomanager.h>
#include <KeywordsPresets/presetkeywordsmodel.h>
#include <Services/artworksupdatehub.h>

#define DECLARE_MODELS_AND_GENERATE(count, withVector) \
    Mocks::CoreTestsEnvironment environment; \
    Models::RecentDirectoriesModel recentDirectories(environment);\
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories);\
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository);\
    artworksListModel.generateAndAddArtworks(count, withVector);

#define MODIFIED_TEST_START\
    QSignalSpy artItemsModifiedSpy(&artworksListModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)));

#define MODIFIED_TEST_END\
    QVERIFY(artItemsModifiedSpy.count() > 0);\
    QVERIFY(artItemsModifiedSpy.takeFirst().at(2).value<QVector<int> >().contains(Models::ArtworksListModel::IsModifiedRole));

void ArtworksListModelTests::removeUnavailableTest() {
    const int count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);

    for (int i = 0; i < count; ++i) {
        if (i%3 == 0) {
            artworksListModel.getArtwork(i)->setUnavailable();
        }
    }

    artworksListModel.purgeUnavailableFiles();

    for (int i = 0; i < artworksListModel.getArtworksSize(); ++i) {
        QVERIFY(!artworksListModel.getArtwork(i)->isUnavailable());
    }
}

void ArtworksListModelTests::unselectAllTest() {
    const int count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);

    for (int i = 0; i < count; ++i) {
        if (i%3 == 0) {
            artworksListModel.getArtwork(i)->setIsSelected(true);
        }
    }

    artworksListModel.unselectAllItems();

    for (int i = 0; i < count; ++i) {
        QVERIFY(!artworksListModel.getArtwork(i)->isSelected());
    }
}

void ArtworksListModelTests::modificationChangesModifiedCountTest() {
    const int count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);

    const int index = 3;

    QCOMPARE(artworksListModel.getModifiedArtworksCount(), 0);
    artworksListModel.getArtwork(index)->setModified();
    QCOMPARE(artworksListModel.getModifiedArtworksCount(), 1);
    artworksListModel.getArtwork(index)->resetModified();
    QCOMPARE(artworksListModel.getModifiedArtworksCount(), 0);
}

void ArtworksListModelTests::removeArtworkDirectorySimpleTest() {
    const int count = 11;
    DECLARE_MODELS_AND_GENERATE(count, false);

    int indexToRemove = 1;

    int firstDirCount = artworksRepository.getFilesCountForDirectory(indexToRemove);

    QCOMPARE(artworksListModel.getArtworksSize(), count);
    artworksListModel.removeFilesFromDirectory(indexToRemove);
    QCOMPARE(artworksListModel.getArtworksSize(), count - firstDirCount);
}

void ArtworksListModelTests::addRemoveOneByOneFewDirsTest() {
    // https://github.com/ribtoks/xpiks/issues/467
    const int count = 2;
    DECLARE_MODELS_AND_GENERATE(count, false);

    QCOMPARE(artworksRepository.getFilesCountForDirectory(0), 1);
    QCOMPARE(artworksRepository.getFilesCountForDirectory(1), 1);
    QCOMPARE(artworksListModel.getArtworksSize(), count);

    artworksListModel.removeFilesFromDirectory(0);
    artworksRepository.cleanupEmptyDirectories();
    QCOMPARE(artworksRepository.rowCount(), 1);

    artworksListModel.removeFilesFromDirectory(0);
    artworksRepository.cleanupEmptyDirectories();
    QCOMPARE(artworksRepository.rowCount(), 0);

    QCOMPARE(artworksListModel.getArtworksSize(), 0);
}

void ArtworksListModelTests::addRemoveOneByOneOneDirTest() {
    // https://github.com/ribtoks/xpiks/issues/467
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);

    QCOMPARE(artworksRepository.getFilesCountForDirectory(0), 1);
    QCOMPARE(artworksListModel.getArtworksSize(), count);

    artworksListModel.removeFilesFromDirectory(0);
    artworksRepository.cleanupEmptyDirectories();
    QCOMPARE(artworksRepository.rowCount(), 0);
    QCOMPARE(artworksListModel.getArtworksSize(), 0);
}

void ArtworksListModelTests::setAllSavedResetsModifiedCountTest() {
    const int count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);
    QVector<int> selectedItems;

    for (int i = 0; i < count; ++i) {
        if (i%3 == 0) {
            artworksListModel.getArtwork(i)->setModified();
            selectedItems.append(i);
        }
    }

    QCOMPARE(artworksListModel.getModifiedArtworksCount(), selectedItems.count());

    artworksListModel.setItemsSaved(Helpers::IndicesRanges(selectedItems));
    QCOMPARE(artworksListModel.getModifiedArtworksCount(), 0);
}

void ArtworksListModelTests::removingLockedArtworksTest() {
    const size_t count = 10;
    DECLARE_MODELS_AND_GENERATE(count, false);

    for (int i = 0; i < (int)count; ++i) {
        artworksListModel.getArtwork(i)->acquire();
    }

    QCOMPARE(artworksListModel.getFinalizationListSize(), (size_t)0);
    artworksListModel.deleteAllItems();
    QCOMPARE(artworksListModel.getFinalizationListSize(), count);
}

void ArtworksListModelTests::plainTextEditToEmptyKeywordsTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    artworksListModel.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");

    artworksListModel.plainTextEdit(0, "")->execute();
    QCOMPARE(artworksListModel.getMockArtwork(0)->getKeywords().length(), 0);
}

void ArtworksListModelTests::plainTextEditToOneKeywordTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    artworksListModel.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");

    QString keywords = "new keyword";
    QStringList result = QStringList() << keywords;

    artworksListModel.plainTextEdit(0, keywords)->execute();
    QCOMPARE(artworksListModel.getMockArtwork(0)->getKeywords(), result);
}

void ArtworksListModelTests::plainTextEditToSeveralKeywordsTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    artworksListModel.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");

    QString keywords = "new keyword, another one, new";
    QStringList result = QStringList() << "new keyword" << "another one" << "new";

    artworksListModel.plainTextEdit(0, keywords)->execute();
    QCOMPARE(artworksListModel.getMockArtwork(0)->getKeywords(), result);
}

void ArtworksListModelTests::plainTextEditToAlmostEmptyTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    artworksListModel.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");

    QString keywords = ",,, , , , , ,,,,   ";
    QStringList result = QStringList();

    artworksListModel.plainTextEdit(0, keywords)->execute();
    QCOMPARE(artworksListModel.getMockArtwork(0)->getKeywords(), result);
}

void ArtworksListModelTests::plainTextEditToMixedTest() {
    const int count = 1;
    DECLARE_MODELS_AND_GENERATE(count, false);
    artworksListModel.getMockArtwork(0)->appendKeywords(QStringList() << "test" << "keywords" << "here");

    QString keywords = ",,, , ,word here , , ,,,,   ";
    QStringList result = QStringList() << "word here";

    artworksListModel.plainTextEdit(0, keywords)->execute();
    QCOMPARE(artworksListModel.getMockArtwork(0)->getKeywords(), result);
}

void ArtworksListModelTests::appendKeywordEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    MODIFIED_TEST_START;

    artworksListModel.appendKeyword(0, "brand new keyword")->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::removeKeywordEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    artworksListModel.appendKeyword(0, "brand new keyword");
    artworksListModel.getMockArtwork(0)->resetModified();

    MODIFIED_TEST_START;

    artworksListModel.removeKeywordAt(0, 0)->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::removeLastKeywordEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    artworksListModel.appendKeyword(0, "brand new keyword");
    artworksListModel.getMockArtwork(0)->resetModified();

    MODIFIED_TEST_START;

    artworksListModel.removeLastKeyword(0)->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::plainTextEditEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    MODIFIED_TEST_START;

    artworksListModel.plainTextEdit(0, "brand,new,keyword")->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::keywordEditEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    artworksListModel.appendKeyword(0, "keyword");
    artworksListModel.getMockArtwork(0)->resetModified();

    MODIFIED_TEST_START;

    artworksListModel.editKeyword(0, 0, "other")->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::pasteKeywordsEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    MODIFIED_TEST_START;

    artworksListModel.pasteKeywords(0, QString("other,keywords,here").split(','))->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::addSuggestedEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    MODIFIED_TEST_START;

    artworksListModel.addSuggestedKeywords(0, QString("suggested,keywords,here").split(','))->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::fillFromQuickBufferEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);
    UndoRedo::UndoRedoManager undoRedoManager;
    Mocks::CommandManagerMock commandManager(undoRedoManager);
    Models::CurrentEditableModel currentEditableModel;
    Models::QuickBuffer quickBuffer(currentEditableModel, commandManager);
    Common::connectTarget<std::shared_ptr<Models::ICurrentEditable>>(currentEditableModel, {artworksListModel});

    quickBuffer.setDescription("description");
    quickBuffer.setTitle("title");
    quickBuffer.setKeywords(QStringList() << "keywords" << "here");

    MODIFIED_TEST_START;

    artworksListModel.setCurrentIndex(0);
    quickBuffer.copyToCurrentEditable();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::addPresetEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);

    KeywordsPresets::PresetKeywordsModel presetKeywordsModel(environment);
    presetKeywordsModel.addItem("preset", QString("preset,keywords").split(','));

    MODIFIED_TEST_START;

    artworksListModel.addPreset(0, 0, presetKeywordsModel)->execute();

    MODIFIED_TEST_END;
}

void ArtworksListModelTests::proxyModelExitEmitsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1, false);
    UndoRedo::UndoRedoManager undoRedoManager;
    Mocks::CommandManagerMock commandManager(undoRedoManager);
    KeywordsPresets::PresetKeywordsModel presetKeywordsModel(environment);
    Services::ArtworksUpdateHub updateHub(artworksListModel);

    Models::ArtworkProxyModel proxyModel(commandManager, presetKeywordsModel, updateHub);

    proxyModel.setSourceArtwork((QObject*)artworksListModel.getMockArtwork(0));
    proxyModel.setDescription("other description");

    artworksListModel.setUpdatesBlocked(false);

    MODIFIED_TEST_START;

    proxyModel.resetModel();

    MODIFIED_TEST_END;
}
