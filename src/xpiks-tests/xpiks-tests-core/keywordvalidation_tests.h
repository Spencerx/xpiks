#ifndef KEYWORDVALIDATIONTESTS_H
#define KEYWORDVALIDATIONTESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class KeywordValidationTests : public QObject
{
    Q_OBJECT
private slots:
    void oneLetterKeywordValidTest();
    void keywordWithSpaceIsValidTest();
    void atSymbolIsValidTest();
    void digitsAreValidTest();
    void simpleKeywordIsValidTest();
    void keywordsFromDigitsAreValidTest();
    void sanitizeKeywordWithBackCommaTest();
    void sanitizeKeywordWithSpacesTest();
    void sanitizeKeywordWithFrontCommaTest();
    void sanitizeKeywordWithAllowedComplexTest();
    void sanitizeOneLetterKeywordTest();
};

#endif // KEYWORDVALIDATIONTESTS_H
