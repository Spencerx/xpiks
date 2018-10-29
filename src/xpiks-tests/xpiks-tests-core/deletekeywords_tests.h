#ifndef DELETEKEYWORDSTESTS_H
#define DELETEKEYWORDSTESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class DeleteKeywordsTests : public QObject
{
    Q_OBJECT
private slots:
    void smokeTest();
    void keywordsCombinedTest();
    void doesNotDeleteOtherCaseTest();
    void doesNotDeleteNoKeywordsTest();
    void deleteCaseInsensitiveTest();
};

#endif // DELETEKEYWORDSTESTS_H
