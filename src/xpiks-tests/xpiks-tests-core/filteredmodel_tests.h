#ifndef FILTEREDMODELTESTS_H
#define FILTEREDMODELTESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class FilteredModelTests : public QObject
{
    Q_OBJECT
private slots:
    void invertSelectionForEmptyTest();
    void invertSelectionForAllSelectedTest();
    void invertSelectionForSingleTest();
    void invertSelectionForHalfSelectedTest();
    void invertSelectionForEvenCountTest();
    void invertSelectionForOddCountTest();
    void removeMetadataMarksAsModifiedTest();
    void removeMetadataDeletesMetadataTest();
    void selectedCountAddTest();
    void selectedCountSubtractTest();
    void findSelectedIndexTest();
    void setCurrentIndexTest();
    void clearKeywordsTest();
    void detachVectorFromSelectedTest();
    void setSelectedForZippingTest();
    void filterDirectoryTest();
    void filterModifiedItemsTest();
    void filterEmptyItemsTest();
    void filterKeywordsUsingAndTest();
    void filterKeywordsUsingOrTest();
    void filterStrictKeywordTest();
    void filterDescriptionTest();
    void filterTitleTest();
    void filterDescriptionAndKeywordsTest();
    void filterTitleAndKeywordsTest();
    void clearEmptyKeywordsMarksModifiedTest();
    void selectExVectorsTest();
    void selectExModifiedTest();
    void selectExAllTest();
    void selectExNoneTest();
    void selectExImagesTest();
};

#endif // FILTEREDMODELTESTS_H
