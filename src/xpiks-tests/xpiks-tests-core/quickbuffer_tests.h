#ifndef QUICKBUFFERTESTS_H
#define QUICKBUFFERTESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class QuickBufferTests: public QObject
{
    Q_OBJECT
private slots:
    void copyArtworkToQuickBufferTest();
    void copyProxyModelToQuickBufferTest();
    void copyCombinedModelToQuickBufferTest();

    void copyHalfEmptyArtworkToQuickBufferTest();
    void copyHalfEmptyProxyModelToQuickBufferTest();
    void copyHalfEmptyCombinedModelToQuickBufferTest();

    void copyKeywordsSuggestorToQuickBufferTest();

    void applyQuickBufferToArtworkTest();
    void applyQuickBufferToProxyModelTest();
    void applyQuickBufferToCombinedModelTest();

    void applyHalfEmptyQuickBufferToArtworkTest();
    void applyHalfEmptyQuickBufferToProxyModelTest();
    void applyHalfEmptyQuickBufferToCombinedModelTest();

    void cannotApplyWhenNoCurrentItemTest();

    void appendRemoveKeywordsTest();
};

#endif // QUICKBUFFERTESTS_H
