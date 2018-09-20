#include "filteredmodel_tests.h"
#include "Mocks/artworkslistmodelmock.h"
#include "Mocks/commandmanagermock.h"
#include "Mocks/artworksrepositorymock.h"
#include "Mocks/coretestsenvironment.h"
#include "Mocks/filescollectionmock.h"
#include <UndoRedo/undoredomanager.h>
#include <KeywordsPresets/presetkeywordsmodel.h>
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Models/Artworks/artworksrepository.h>
#include <Models/Connectivity/ziparchiver.h>
#include <Models/Session/recentdirectoriesmodel.h>
#include <Models/settingsmodel.h>

#define DECLARE_MODELS \
    Mocks::CoreTestsEnvironment environment;\
    UndoRedo::UndoRedoManager undoRedoManager;\
    Mocks::CommandManagerMock commandManager(undoRedoManager);\
    Models::RecentDirectoriesModel recentDirectories(environment);\
    recentDirectories.initialize();\
    Mocks::ArtworksRepositoryMock artworksRepository(recentDirectories);\
    Mocks::ArtworksListModelMock artworksListModel(artworksRepository);\
    Models::SettingsModel settingsModel(environment); \
    settingsModel.initializeConfigs(); \
    settingsModel.setSearchUsingAnd(false);\
    KeywordsPresets::PresetKeywordsModel keywordsPresets(environment);\
    Models::FilteredArtworksListModel filteredItemsModel(\
    artworksListModel, commandManager, keywordsPresets, settingsModel);

#define DECLARE_MODELS_AND_GENERATE(count)\
    DECLARE_MODELS\
    artworksListModel.generateAndAddArtworks(count);

void FilteredModelTests::invertSelectionForEmptyTest(){
    DECLARE_MODELS_AND_GENERATE(10);

    int allItemsCount = filteredItemsModel.getItemsCount();

    filteredItemsModel.invertSelectionArtworks();

    int selected = filteredItemsModel.retrieveNumberOfSelectedItems();
    QCOMPARE(selected, allItemsCount);
}

void FilteredModelTests::invertSelectionForAllSelectedTest() {
    DECLARE_MODELS_AND_GENERATE(10);

    int allItemsCount = filteredItemsModel.getItemsCount();
    for (int i = 0; i < allItemsCount; i++) {
        artworksListModel.getMockArtwork(i)->setIsSelected(true);
    }

    filteredItemsModel.invertSelectionArtworks();

    int selected = filteredItemsModel.retrieveNumberOfSelectedItems();
    QCOMPARE(selected, 0);
}

void FilteredModelTests::invertSelectionForSingleTest(){
    DECLARE_MODELS_AND_GENERATE(10);

    int allItemsCount = filteredItemsModel.getItemsCount();

    artworksListModel.getMockArtwork(0)->setIsSelected(true);

    filteredItemsModel.invertSelectionArtworks();

    int selected = filteredItemsModel.retrieveNumberOfSelectedItems();
    QCOMPARE(selected, (allItemsCount-1));
}

void FilteredModelTests::invertSelectionForHalfSelectedTest(){
    DECLARE_MODELS_AND_GENERATE(10);

    int allItemsCount = filteredItemsModel.getItemsCount();
    for (int i =0; i < allItemsCount; i++) {
        artworksListModel.getMockArtwork(i)->setIsSelected(i < allItemsCount/2);
    }

    filteredItemsModel.invertSelectionArtworks();

    int selected = filteredItemsModel.retrieveNumberOfSelectedItems();
    QCOMPARE(selected, (allItemsCount - allItemsCount/2));
}

void FilteredModelTests::invertSelectionForEvenCountTest() {
    DECLARE_MODELS_AND_GENERATE(10);

    int allItemsCount = filteredItemsModel.getItemsCount();
    for (int i = 0; i < allItemsCount; i++) {
        artworksListModel.getMockArtwork(i)->setIsSelected(i < allItemsCount/3);
    }

    filteredItemsModel.invertSelectionArtworks();

    int selected = filteredItemsModel.retrieveNumberOfSelectedItems();
    QCOMPARE(selected, (allItemsCount - allItemsCount/3));
}

void FilteredModelTests::invertSelectionForOddCountTest(){
    DECLARE_MODELS_AND_GENERATE(11);

    int allItemsCount = filteredItemsModel.getItemsCount();
    for (int i =0; i<allItemsCount; i++) {
        artworksListModel.getMockArtwork(i)->setIsSelected(i < allItemsCount/3);
    }

    filteredItemsModel.invertSelectionArtworks();

    int selected = filteredItemsModel.retrieveNumberOfSelectedItems();
    QCOMPARE(selected, (allItemsCount - allItemsCount/3));
}

void FilteredModelTests::removeMetadataMarksAsModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1);

    auto artwork = artworksListModel.getMockArtwork(0);

    QVERIFY(!artwork->isModified());

    artwork->set("title", "description", QStringList() << "keyword1" << "keyword2");
    artwork->setIsSelected(true);
    filteredItemsModel.removeMetadataInSelected();

    QVERIFY(artwork->isModified());
}

void FilteredModelTests::removeMetadataDeletesMetadataTest() {
    DECLARE_MODELS_AND_GENERATE(10);

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);
        artwork->set("title", "description", QStringList() << "keyword1" << "keyword2");
        artwork->setIsSelected(true);
    }

    filteredItemsModel.removeMetadataInSelected();

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);
        QVERIFY(artwork->isDescriptionEmpty());
        QVERIFY(artwork->isTitleEmpty());
        QVERIFY(artwork->areKeywordsEmpty());
        QVERIFY(artwork->isModified());
    }
}

void FilteredModelTests::selectedCountAddTest() {
    DECLARE_MODELS_AND_GENERATE(10);
    QObject::connect(&artworksListModel, &Models::ArtworksListModel::artworkSelectedChanged,
                     &filteredItemsModel, &Models::FilteredArtworksListModel::itemSelectedChanged);

    QCOMPARE(filteredItemsModel.getSelectedArtworksCount(), 0);

    for (int i = 0; i < 10; i += 2) {
        auto artwork = artworksListModel.getMockArtwork(i);
        artwork->setIsSelected(true);
    }

    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    QCOMPARE(filteredItemsModel.getSelectedArtworksCount(), 5);
}

void FilteredModelTests::selectedCountSubtractTest() {
    DECLARE_MODELS_AND_GENERATE(10);
    QObject::connect(&artworksListModel, &Models::ArtworksListModel::artworkSelectedChanged,
                     &filteredItemsModel, &Models::FilteredArtworksListModel::itemSelectedChanged);

    QCOMPARE(filteredItemsModel.getSelectedArtworksCount(), 0);

    filteredItemsModel.selectFilteredArtworks();

    for (int i = 0; i < 10; i += 2) {
        auto artwork = artworksListModel.getMockArtwork(i);
        artwork->setIsSelected(false);
    }

    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    QCOMPARE(filteredItemsModel.getSelectedArtworksCount(), 5);
}

void FilteredModelTests::findSelectedIndexTest() {
    DECLARE_MODELS_AND_GENERATE(10);
    artworksListModel.getMockArtwork(4)->setIsSelected(true);
    int index = filteredItemsModel.findSelectedItemIndex();
    QCOMPARE(index, 4);

    artworksListModel.getMockArtwork(9)->setIsSelected(true);
    index = filteredItemsModel.findSelectedItemIndex();
    QCOMPARE(index, -1);
}

void FilteredModelTests::clearKeywordsTest() {
    DECLARE_MODELS_AND_GENERATE(1);

    auto artwork = artworksListModel.getMockArtwork(0);
    artwork->set("title", "description", QStringList() << "keyword1" << "keyword2");

    filteredItemsModel.clearKeywords(0);

    QVERIFY(!artwork->isDescriptionEmpty());
    QVERIFY(!artwork->isTitleEmpty());
    QVERIFY(artwork->areKeywordsEmpty());
    QVERIFY(artwork->isModified());
}

void FilteredModelTests::detachVectorFromSelectedTest() {
    DECLARE_MODELS_AND_GENERATE(10);

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);
        artwork->set("title", "description", QStringList() << "keyword1" << "keyword2");
        artwork->attachVector(QString(artwork->getFilepath()).replace(".jpg", ".eps"));
        artwork->setIsSelected(true);
    }

    filteredItemsModel.detachVectorFromSelected();

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);
        QVERIFY(!artwork->isModified());
        QVERIFY(!artwork->hasVectorAttached());
    }
}

void FilteredModelTests::setSelectedForZippingTest() {
    DECLARE_MODELS_AND_GENERATE(10);
    Models::ZipArchiver zipArchiver;

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);
        artwork->set("title", "description", QStringList() << "keyword1" << "keyword2");
        artwork->attachVector("/path/to/random/vector.eps");

        if (i % 2) {
            artwork->setIsSelected(true);
        }
    }

    zipArchiver.setArtworks(filteredItemsModel.getSelectedArtworks());

    auto &snapshot = zipArchiver.getArtworksSnapshot();
    QCOMPARE((int)snapshot.size(), 5);
}

void FilteredModelTests::filterModifiedItemsTest() {
    DECLARE_MODELS_AND_GENERATE(10);

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);
        artwork->set("title", "description", QStringList() << "keyword1" << "keyword2");

        if (i % 2) {
            artwork->setModified();
        }
    }

    filteredItemsModel.setSearchTerm("x:modified");
    QCOMPARE(filteredItemsModel.getItemsCount(), 5);
}

void FilteredModelTests::filterEmptyItemsTest() {
    DECLARE_MODELS_AND_GENERATE(10);

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);

        if (i % 2) {
            artwork->set("title", "description", QStringList() << "keyword1" << "keyword2");
        } else {
            artwork->set("", "", QStringList());
        }
    }

    filteredItemsModel.setSearchTerm("x:empty");
    QCOMPARE(filteredItemsModel.getItemsCount(), 5);
}

void FilteredModelTests::filterKeywordsUsingAndTest() {
    DECLARE_MODELS_AND_GENERATE(10);
    settingsModel.setSearchUsingAnd(true);

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);

        if (i % 2 == 0) {
            artwork->set("title", "description", QStringList() << "keyword1" << "keyword2" << "mess2");
            if (i % 4 == 0) {
                artwork->setModified();
            }
        } else {
            artwork->set("", "", QStringList());
        }
    }

    filteredItemsModel.setSearchTerm("keyword1 keyword2");
    QCOMPARE(filteredItemsModel.getItemsCount(), 5);

    filteredItemsModel.setSearchTerm("keyword1 keyword2 x:modified");
    QCOMPARE(filteredItemsModel.getItemsCount(), 3);
}

void FilteredModelTests::filterKeywordsUsingOrTest() {
    DECLARE_MODELS_AND_GENERATE(10);

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);

        if (i % 2 == 0) {
            artwork->set("title", "description", QStringList() << "keyword1" << "mess1");
        } else {
            artwork->set("title", "description", QStringList() << "keyword2" << "mess2");
        }
    }

    filteredItemsModel.setSearchTerm("keyword1");
    QCOMPARE(filteredItemsModel.getItemsCount(), 5);

    filteredItemsModel.setSearchTerm("keyword1 keyword2");
    QCOMPARE(filteredItemsModel.getItemsCount(), 10);
}

void FilteredModelTests::filterStrictKeywordTest() {
    DECLARE_MODELS_AND_GENERATE(10);

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);

        if (i % 2 == 0) {
            artwork->set("title", "description", QStringList() << "keyword1" << "mess1");
        } else {
            artwork->set("title", "description", QStringList() << "keyword2" << "mess2");
        }
    }

    filteredItemsModel.setSearchTerm("keyword");
    QCOMPARE(filteredItemsModel.getItemsCount(), 10);

    filteredItemsModel.setSearchTerm("!keyword");
    QCOMPARE(filteredItemsModel.getItemsCount(), 0);

    filteredItemsModel.setSearchTerm("!keyword1");
    QCOMPARE(filteredItemsModel.getItemsCount(), 5);
}

void FilteredModelTests::filterDescriptionTest() {
    DECLARE_MODELS_AND_GENERATE(10);

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);

        if (i % 2) {
            artwork->set("title", "description", QStringList() << "keyword1" << "keyword2");
        } else {
            artwork->set("", "", QStringList());
        }
    }

    filteredItemsModel.setSearchTerm("desc");
    QCOMPARE(filteredItemsModel.getItemsCount(), 5);

    filteredItemsModel.setSearchTerm("!desc");
    QCOMPARE(filteredItemsModel.getItemsCount(), 0);

    filteredItemsModel.setSearchTerm("description");
    QCOMPARE(filteredItemsModel.getItemsCount(), 5);

    // strict match does not work for description/title
    filteredItemsModel.setSearchTerm("!description");
    QCOMPARE(filteredItemsModel.getItemsCount(), 0);
}

void FilteredModelTests::filterTitleTest() {
    DECLARE_MODELS_AND_GENERATE(10);

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);

        if (i % 2) {
            artwork->set("title", "description", QStringList() << "keyword1" << "keyword2");
        } else {
            artwork->set("", "", QStringList());
        }
    }

    filteredItemsModel.setSearchTerm("tit");
    QCOMPARE(filteredItemsModel.getItemsCount(), 5);

    filteredItemsModel.setSearchTerm("!tit");
    QCOMPARE(filteredItemsModel.getItemsCount(), 0);

    filteredItemsModel.setSearchTerm("title");
    QCOMPARE(filteredItemsModel.getItemsCount(), 5);

    // strict match does not work for description/title
    filteredItemsModel.setSearchTerm("!title");
    QCOMPARE(filteredItemsModel.getItemsCount(), 0);
}

void FilteredModelTests::filterDescriptionAndKeywordsTest() {
    DECLARE_MODELS_AND_GENERATE(10);

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);

        if (i % 2 == 0) {
            artwork->set("title", "description", QStringList() << "keyword1" << "mess1");
        } else {
            artwork->set("title", "description", QStringList() << "keyword2" << "mess2");
        }
    }

    filteredItemsModel.setSearchTerm("keyword1 desc");
    QCOMPARE(filteredItemsModel.getItemsCount(), 10);

    filteredItemsModel.setSearchTerm("!keyword desc");
    QCOMPARE(filteredItemsModel.getItemsCount(), 10);

    settingsModel.setSearchUsingAnd(true);

    filteredItemsModel.setSearchTerm("!keyword desc");
    QCOMPARE(filteredItemsModel.getItemsCount(), 0);

    filteredItemsModel.setSearchTerm("!keyword2 description");
    QCOMPARE(filteredItemsModel.getItemsCount(), 5);
}

void FilteredModelTests::filterTitleAndKeywordsTest() {
    DECLARE_MODELS_AND_GENERATE(10);

    for (int i = 0; i < 10; ++i) {
        auto artwork = artworksListModel.getMockArtwork(i);

        if (i % 2 == 0) {
            artwork->set("title", "description", QStringList() << "keyword1" << "mess1");
        } else {
            artwork->set("title", "description", QStringList() << "keyword2" << "mess2");
        }
    }

    filteredItemsModel.setSearchTerm("keyword1 tit");
    QCOMPARE(filteredItemsModel.getItemsCount(), 10);

    filteredItemsModel.setSearchTerm("!keyword tit");
    QCOMPARE(filteredItemsModel.getItemsCount(), 10);

    settingsModel.setSearchUsingAnd(true);

    filteredItemsModel.setSearchTerm("!keyword tit");
    QCOMPARE(filteredItemsModel.getItemsCount(), 0);

    filteredItemsModel.setSearchTerm("!keyword2 title");
    QCOMPARE(filteredItemsModel.getItemsCount(), 5);
}

void FilteredModelTests::clearEmptyKeywordsMarksModifiedTest() {
    DECLARE_MODELS_AND_GENERATE(1);
    auto artwork = artworksListModel.getMockArtwork(0);
    artwork->clearKeywords();
    artwork->resetModified();

    filteredItemsModel.clearKeywords(0);
    QVERIFY(!artwork->isModified());
}

#define ADD_DIFFERENT_FILES \
    auto files = std::make_shared<Mocks::FilesCollectionMock>();\
    files->add({"image.jpg", Filesystem::ArtworkFileType::Image});\
    files->add({"vector.jpg", Filesystem::ArtworkFileType::Image});\
    files->add({"vector.eps", Filesystem::ArtworkFileType::Vector});\
    files->add({"video.mp4", Filesystem::ArtworkFileType::Video});\
    artworksListModel.addFiles(files, Common::AddFilesFlags::None);

#define CHECK_ALL_SELECTED(value)\
    artworksListModel.foreachArtwork([](int index, std::shared_ptr<Mocks::ArtworkMetadataMock> const &artwork) {\
        QVERIFY2(artwork->isSelected() == value, QString("Artwork selection is different at %1").arg(index).toStdString().data());\
    });

#define CHECK_ONLY_SELECTED(i)\
    artworksListModel.foreachArtwork([=](int index, std::shared_ptr<Mocks::ArtworkMetadataMock> const &artwork) {\
        QVERIFY2(artwork->isSelected() == (index == i), QString("Artwork selection is different at %1").arg(index).toStdString().data());\
    });

void FilteredModelTests::selectExVectorsTest() {
    DECLARE_MODELS;
    ADD_DIFFERENT_FILES;

    CHECK_ALL_SELECTED(false);

    filteredItemsModel.selectArtworksEx(Models::FilteredArtworksListModel::SelectVectors);

    CHECK_ONLY_SELECTED(1);
}

void FilteredModelTests::selectExModifiedTest() {
    DECLARE_MODELS;
    ADD_DIFFERENT_FILES;

    CHECK_ALL_SELECTED(false);

    artworksListModel.getMockArtwork(2)->setTitle("other title 123214");

    filteredItemsModel.selectArtworksEx(Models::FilteredArtworksListModel::SelectModified);

    CHECK_ONLY_SELECTED(2);
}

void FilteredModelTests::selectExAllTest() {
    DECLARE_MODELS;
    ADD_DIFFERENT_FILES;

    CHECK_ALL_SELECTED(false);

    filteredItemsModel.selectArtworksEx(Models::FilteredArtworksListModel::SelectAll);

    CHECK_ALL_SELECTED(true);
}

void FilteredModelTests::selectExNoneTest() {
    DECLARE_MODELS;
    ADD_DIFFERENT_FILES;

    CHECK_ALL_SELECTED(false);

    artworksListModel.getMockArtwork(0)->setIsSelected(true);
    artworksListModel.getMockArtwork(2)->setIsSelected(true);

    filteredItemsModel.selectArtworksEx(Models::FilteredArtworksListModel::SelectNone);

    CHECK_ALL_SELECTED(false);
}

void FilteredModelTests::selectExImagesTest() {
    DECLARE_MODELS;
    ADD_DIFFERENT_FILES;

    CHECK_ALL_SELECTED(false);

    filteredItemsModel.selectArtworksEx(Models::FilteredArtworksListModel::SelectImages);

    QVERIFY2(artworksListModel.getMockArtwork(0)->isSelected(), "Image is not selected");
    QVERIFY2(!artworksListModel.getMockArtwork(1)->isSelected(), "Vector is selected");

    QEXPECT_FAIL("", "for now all of the items are images", Continue);
    QVERIFY2(!artworksListModel.getMockArtwork(2)->isSelected(), "Video is selected");
}
