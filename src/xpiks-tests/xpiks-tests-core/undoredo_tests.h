#ifndef UNDOREDOTESTS_H
#define UNDOREDOTESTS_H

#include <QObject>
#include <QtTest/QtTest>

class UndoRedoTests : public QObject
{
    Q_OBJECT
private slots:
    void undoAddCommandTest();
    void undoRemoveItemsTest();
    void undoRemoveAddFullDirectoryTest();
    void undoRemoveNotFullDirectoryTest();
    void undoRemoveLaterFullDirectoryTest();
    void undoModifyCommandTest();
    void undoPasteCommandTest();
    void undoClearAllTest();
    void undoClearKeywordsTest();
    void undoReplaceCommandTest();
};

#endif // UNDOREDOTESTS_H
