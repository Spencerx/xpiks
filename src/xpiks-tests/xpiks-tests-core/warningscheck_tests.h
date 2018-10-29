#ifndef WARNINGSCHECKTESTS_H
#define WARNINGSCHECKTESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class WarningsCheckTests : public QObject
{
    Q_OBJECT
private slots:
    void emptyKeywordsTest();
    void tooFewKeywordsTest();
    void appendingKeywordChangesWarningsTest();
    void spellingKeywordsTest();
    void spellingKeywordsChangesWhenRemovedTest();
    void spellingDescriptionTest();
    void spellingDescriptionChangesTest();
    void emptyDescriptionTest();
    void descriptionLengthChangesTest();
    void descriptionTooBigTest();
    void emptyTitleTest();
    void titleLengthChangesTest();
    void spellingTitleTest();
    void spellingTitleChangesWhenRemovedTest();
    void keywordsInDescriptionTest();
    void keywordsInTitleTest();
    void titleTooBigTest();
    void descriptionTooBigChangesTest();
};

#endif // WARNINGSCHECKTESTS_H
