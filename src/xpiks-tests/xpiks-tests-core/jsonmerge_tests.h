#ifndef JSONMERGE_TESTS_H
#define JSONMERGE_TESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class JsonMergeTests: public QObject
{
    Q_OBJECT
private slots:
    void mergeArraysOfObjectsTest();
    void mergeAddArraysOfObjectsTest();
    void mergeArraysOfStringsTest();
    void mergeOverwriteTest();
    void mergeExistingElementsTest();
    void mergeSelfTest();
};

#endif // JSONMERGE_TESTS_H
