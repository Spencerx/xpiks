#ifndef ADDCOMMAND_TESTS_H
#define ADDCOMMAND_TESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class AddCommandTests : public QObject
{
    Q_OBJECT
private slots:
    void addNoArtworksToEmptyRepositoryTest();
    void addOneArtworkToEmptyRepositoryTest();
    void addAndAttachVectorsTest();
    void addAndAttachVectorsLaterTest();
    void addAndDontAttachVectorsOtherDirTest();
    void addAndDontAttachVectorsEmptyDirTest();
    void addAndDontAttachVectorsStartsWithTest();
    void addAndAttachFromSingleDirectoryTest();
    void addSingleDirectoryAndAttachLaterTest();
};

#endif // ADDCOMMAND_TESTS_H
