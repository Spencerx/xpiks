#ifndef UPLOADINFOREPOSITORY_TESTS_H
#define UPLOADINFOREPOSITORY_TESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class UploadInfoRepositoryTests: public QObject
{
    Q_OBJECT
private slots:
    void parseJsonTest();
    void jsonUpdatesImagesDirTest();
    void updateFtpHostByIDTest();
    void idWasUpdatedForFullMatchTest();
    void idNotUpdatedForSomeMatchTest();
    void zipRequiredTest();
    void selectedInfosCountTest();
    void removeItemTest();
    void missingDetailsTest();
    void notFullUploadInfoTest();
    void findByHostTest();
    void setPasswordIsEncodedTest();
    void initAccountsWithCorrectMPTest();
    void initAccountsWithWrongMPTest();
    void finalizeAccountsRestoresPasswordTest();
    void finalizeAccountsNoEffectTest();
    void defaultMasterPasswordChangeTest();
    void masterPasswordChangeTest();
    void defaultMasterPasswordResetTest();
    void masterPasswordResetTest();
    void validateNonAutocompletedHostTest();
};

#endif // UPLOADINFOREPOSITORY_TESTS_H
