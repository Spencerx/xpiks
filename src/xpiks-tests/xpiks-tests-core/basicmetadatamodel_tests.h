#ifndef BASICKEYWORDSMODELTESTS_H
#define BASICKEYWORDSMODELTESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

#include "Services/SpellCheck/spellcheckinfo.h"

class BasicKeywordsModelTests: public QObject
{
    Q_OBJECT
private slots:
    void constructEmptyTest();
    void simpleAddKeywordTest();
    void simpleSetKeywordsTest();
    void addExistingKeywordTest();
    void addSeveralKeywordsTest();
    void removeExistingKeywordTest();
    void removeNonExistingKeywordTest();
    void appendSameKeywordsTest();
    void appendSameChangedKeywordTest();
    void appendNoKeywordsTest();
    void expandPresetTest();
    void clearKeywordsTest();
    void clearModelTest();
    void containsKeywordTest();
    void containsKeywordStrictTest();
    void containsKeywordFuzzyTest();
    void doesNotContainKeywordTest();
    void setSameTitleTest();
    void setSameDescriptionTest();
    void editKeywordToAnotherTest();
    void editKeywordToSameTest();
    void editKeywordAnotherCaseTest();
    void addRemoveAddUpperCaseWordTest();
    void editToUpperCaseTest();
    void hasKeywordTest();
    void simpleReplaceTest();
    void descriptionReplaceTest();
    void titleReplaceTest();
    void keywordsReplaceTest();
    void noReplaceCaseSensitiveTest();
    void replaceCaseSensitiveTest();
    void replaceWholeWordsTest();
    void replaceKeywordsWithRemoveTest();
    void removeKeywordsFromSetTest();
    void noneKeywordsRemovedFromSetTest();
    void removeKeywordsCaseSensitiveTest();

private:
    SpellCheck::SpellCheckInfo m_SpellCheckInfo;
};

#endif // BASICKEYWORDSMODELTESTS_H
