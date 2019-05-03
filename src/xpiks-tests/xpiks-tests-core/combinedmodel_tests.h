#ifndef COMBINEDMODELTESTS_H
#define COMBINEDMODELTESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class CombinedModelTests : public QObject {
    Q_OBJECT

public:
    CombinedModelTests(QObject *parent = nullptr);

private slots:
    void trivialCombineNoItemsTest();
    void trivialCombineOneItemTest();
    void combineAllEmptyTest();
    void combineSeveralSameItemsTest();
    void combineSeveralWithEmptyFirstTest();
    void combineSeveralWithEmptyManyTest();
    void combineSeveralWithEmptyTest();
    void combineWithSeveralEmptyTest();
    void combineAllDifferentItemsTest();
    void combineAllManyDifferentItemsTest();
    void combineAllManyLastDifferentItemsTest();
    void combineCommonInKeywordsTest();
    void combineCommonInManyKeywordsTest();
    void combineCommonInTitleTest();
    void combineCommonInDescriptionTest();
    void editSeveralWithSameKeywordsTest();
    void recombineAfterRemoveDifferentTest();
    void recombineAfterRemoveAllButOneTest();
    void recombineAfterChangesTest();
    void isNotModifiedAfterTitleDescEditTest();
    void isModifiedAfterKeywordsAppendTest();
    void isModifiedAfterKeywordRemovalTest();
    void isModifiedAfterKeywordEditTest();
    void isModifiedAfterKeywordsClearTest();
    void isNotModifiedAfterEmptyKeywordsClearTest();
    void isModifiedStatusNotResetWithOtherTest();
    void initArtworksEmitsRowsInsertTest();
    void initEmptyArtworksDoesNotEmitTest();
    void initOneArtworkEnablesAllFields();
    void initManyArtworksDoesNotEnableAllFields();
    void resetModelClearsEverythingTest();
    void appendNewKeywordEmitsCountChangedTest();
    void appendExistingKeywordDoesNotEmitTest();
    void pasteNewKeywordsEmitsCountChangedTest();
    void pasteExistingKeywordsDoesNotEmitTest();
    void editKeywordDoesNotEmitCountChangedTest();
    void notSavedAfterAllDisabledTest();
    void notSavedAfterNothingModifiedTest();
    void notSavedAfterModifiedDisabledTest();
    void savedAfterModifiedDescriptionTest();
    void savedAfterModifiedTitleTest();
    void savedAfterKeywordsModifiedTest();
    void nothingChangedIfNoModificationsMadeTest();
    void caseIsPreservedForOneItemTest();
    void caseIsPreservedForSeveralItemsTest();
    void clearKeywordsFiresKeywordsCountTest();
};

#endif // COMBINEDMODELTESTS_H
