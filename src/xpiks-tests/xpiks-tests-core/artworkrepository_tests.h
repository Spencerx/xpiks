#ifndef ARTWORKREPOSITORYTESTS_H
#define ARTWORKREPOSITORYTESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class ArtworkRepositoryTests : public QObject
{
    Q_OBJECT
private slots:
    void simpleAccountFileTest();
    void accountSameFileTest();
    void addFilesFromOneDirectoryTest();
    void addAndRemoveSameFileTest();
    void removeNotExistingFileTest();
    void brandNewDirectoriesCountTest();
    void differentNewDirectoriesCountTest();
    void newFilesCountTest();
    void noNewDirectoriesCountTest();
    void noNewFilesCountTest();
    void accountFileEmitsTest();
    void selectFolderTest();
    void oneEmptyDirectoryStaysTest();
    void fewEmptyDirectoriesStayTest();
    void removeDirectoryWithHighIDNumberTest();
    // selection tests
    void allDirsInitiallySelectedTest();
    void unselectOneSelectsOnlyOneTest();
    void cannotUnselectTheOnlyOneTest();
    void toggleSameDirectoryTwiceAllSelectedTest();
    void unselectLastSelectedSelectsAllTest();
    void removeOnlySelectedSelectsAllTest();
    void removeOneOfFewSelectedStaysSameTest();
    void undoRemoveOnlySelectedSelectsItTest();
    void undoRemoveOneOfFewSelectedTest();
    void undoRemoveAfterUserSelectsOtherTest();
    void undoRemoveAfterUserSelectsFewTest();
    void undoRemoveOfTheOnlyOneSelectsItTest();
    void undoRemoveAfterAllOtherSelectedTest();
};

#endif // ARTWORKREPOSITORYTESTS_H
