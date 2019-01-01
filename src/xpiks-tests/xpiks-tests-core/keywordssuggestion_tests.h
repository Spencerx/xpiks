#ifndef KEYWORDSSUGGESTION_TESTS_H
#define KEYWORDSSUGGESTION_TESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class KeywordsSuggestionTests : public QObject
{
    Q_OBJECT
private slots:
    void suggestSingleKeywordsSetTest();
    void suggestKeywordsSimpleTest();
    void repeatableSuggestionTest();
};

#endif // KEYWORDSSUGGESTION_TESTS_H
