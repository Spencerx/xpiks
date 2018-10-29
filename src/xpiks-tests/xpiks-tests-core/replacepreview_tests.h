#ifndef REPLACEPREVIEW_H
#define REPLACEPREVIEW_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class ReplacePreviewTests: public QObject
{
    Q_OBJECT
private slots:
    void smokeTest();
    void simpleEntryTest();
    void overlappingEntryTest();
    void advancedEntryTest();
    void hitInTheMiddleTest();
};

#endif // REPLACEPREVIEW_H
