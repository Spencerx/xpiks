#include "uploadinforepository_tests.h"

#include <Encryption/secretsmanager.h>
#include <Helpers/asynccoordinator.h>
#include <Microstocks/stockftpoptions.h>
#include <Models/Connectivity/uploadinforepository.h>
#include <Models/Connectivity/uploadinfo.h>

#include "Mocks/coretestsenvironment.h"
#include "Mocks/requestsservicemock.h"

void UploadInfoRepositoryTests::parseJsonTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);
    const char *stocksJson = R"JSON(
    {
        "overwrite": false,
        "stocks_ftp": [
            {
                "name": "Shutterstock",
                "ftp": "ftp.shutterstock.com"
            },
            {
                "id": 12345,
                "name": "Depositphotos",
                "ftp": "ftp.depositphotos.com",
                "zip_vector": true
            },
            {
                "name": "BigStock",
                "ftp": "ftp.bigstockphoto.com"
            },
            {
                "name": "ColourBox",
                "ftp": "ftp.skyfish.com",
                "images_dir": "stock/images",
                "vectors_dir": "stock/vector",
                "videos_dir": "stock/video"
            },
            {
                "id": 9876,
                "name": "Alamy",
                "ftp": "ftp://upload.alamy.com",
                "images_dir": "Stock",
                "vectors_dir": "Vector"
            },
            {
                "name": "Crestock",
                "ftp": "ftp.crestock.com"
            }
        ]
    }
)JSON";

    QByteArray jsonData(stocksJson);
    uploadInfos.accessStocksList().setRemoteOverride(jsonData);

    Helpers::AsyncCoordinator coordinator;
    Mocks::RequestsServiceMock requestsService;
    uploadInfos.initializeStocksList(coordinator, requestsService);

    auto &stocks = uploadInfos.accessStocksList();
    QCOMPARE(stocks.getStockNamesList().size(), 6);
    QCOMPARE(stocks.findFtpOptionsByTitle("Crestock")->m_FtpAddress, QString("ftp.crestock.com"));
    QCOMPARE(stocks.findFtpOptionsByTitle("ColourBox")->m_VectorsDir, QString("stock/vector"));
    QCOMPARE(stocks.findFtpOptionsByTitle("Depositphotos")->m_ZipVector, true);
    QCOMPARE(stocks.findFtpOptionsByTitle("Shutterstock")->m_ZipVector, false);
    QCOMPARE(stocks.findFtpOptionsByTitle("Alamy")->m_ImagesDir, QString("Stock"));
    QVERIFY(stocks.findFtpOptionsByTitle("BigStock")->m_ImagesDir.isEmpty());
    QCOMPARE(stocks.findFtpOptionsByTitle("Depositphotos")->m_ID, 12345);
    QCOMPARE(stocks.findFtpOptionsByID(9876)->m_Title, QString("Alamy"));
}

void UploadInfoRepositoryTests::jsonUpdatesImagesDirTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);
    const char *stocksJson = R"JSON(
    {
        "overwrite": false,
        "stocks_ftp": [
            {
                "name": "Shutterstock",
                "ftp": "ftp.shutterstock.com"
            },
            {
                "name": "Alamy",
                "ftp": "ftp://upload.alamy.com",
                "images_dir": "Stock",
                "vectors_dir": "Vector"
            }
        ]
    }
)JSON";

    const QString oldHost = "upload.alamy.com";
    auto item1 = uploadInfos.appendItem();
    item1->setTitle("Alamy");
    item1->setHost(oldHost);
    item1->setImagesDir("random dir here");
    item1->setVectorsDir("another random");

    QByteArray jsonData(stocksJson);
    uploadInfos.accessStocksList().setRemoteOverride(jsonData);

    Helpers::AsyncCoordinator coordinator;
    Mocks::RequestsServiceMock requestsService;
    uploadInfos.initializeStocksList(coordinator, requestsService);

    QCOMPARE(item1->getImagesDir(), QString("Stock"));
    QCOMPARE(item1->getVectorsDir(), QString("Vector"));
    QCOMPARE(item1->getHost(), oldHost);
}

void UploadInfoRepositoryTests::updateFtpHostByIDTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);
    const char *stocksJson = R"JSON(
    {
        "overwrite": false,
        "stocks_ftp": [
            {
                "name": "Alamy",
                "id": 1234,
                "ftp": "ftp://upload.alamy.com",
                "images_dir": "Stock",
                "vectors_dir": "Vector"
            }
        ]
    }
)JSON";

    auto item1 = uploadInfos.appendItem();
    item1->setTitle("Alamy");
    item1->setID(1234);
    item1->setHost("upload.alaaaaamy.com");
    item1->setImagesDir("random dir here");
    item1->setVectorsDir("another random");

    QByteArray jsonData(stocksJson);
    uploadInfos.accessStocksList().setRemoteOverride(jsonData);

    Helpers::AsyncCoordinator coordinator;
    Mocks::RequestsServiceMock requestsService;
    uploadInfos.initializeStocksList(coordinator, requestsService);

    QCOMPARE(item1->getImagesDir(), QString("Stock"));
    QCOMPARE(item1->getVectorsDir(), QString("Vector"));
    QCOMPARE(item1->getHost(), QString("ftp://upload.alamy.com"));
}

void UploadInfoRepositoryTests::idWasUpdatedForFullMatchTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);
    const char *stocksJson = R"JSON(
    {
        "overwrite": false,
        "stocks_ftp": [
            {
                "name": "Alamy",
                "id": 1234,
                "ftp": "ftp://upload.alamy.com"
            }
        ]
    }
)JSON";

    auto item1 = uploadInfos.appendItem();
    item1->setTitle("Alamy");
    item1->setID(0);
    item1->setHost("ftp://upload.alamy.com");

    QByteArray jsonData(stocksJson);
    uploadInfos.accessStocksList().setRemoteOverride(jsonData);

    Helpers::AsyncCoordinator coordinator;
    Mocks::RequestsServiceMock requestsService;
    uploadInfos.initializeStocksList(coordinator, requestsService);

    QCOMPARE(item1->getID(), 1234);
}

void UploadInfoRepositoryTests::idNotUpdatedForSomeMatchTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);
    const char *stocksJson = R"JSON(
    {
        "overwrite": false,
        "stocks_ftp": [
            {
                "name": "Alamy",
                "id": 1234,
                "ftp": "ftp://upload.alamy.com"
            }
        ]
    }
)JSON";

    auto item1 = uploadInfos.appendItem();
    // title does not match in case
    item1->setTitle("alamy");
    item1->setID(0);
    item1->setHost("ftp://upload.alamy.com");

    QByteArray jsonData(stocksJson);
    uploadInfos.accessStocksList().setRemoteOverride(jsonData);

    Helpers::AsyncCoordinator coordinator;
    Mocks::RequestsServiceMock requestsService;
    uploadInfos.initializeStocksList(coordinator, requestsService);

    QCOMPARE(item1->getID(), 0);
}

void UploadInfoRepositoryTests::zipRequiredTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    auto item1 = uploadInfos.appendItem();
    item1->setIsSelected(true);

    auto item2 = uploadInfos.appendItem();
    item2->setZipBeforeUpload(true);

    QCOMPARE(uploadInfos.isZippingRequired(), false);

    item2->setIsSelected(true);
    QCOMPARE(uploadInfos.isZippingRequired(), true);
}

void UploadInfoRepositoryTests::selectedInfosCountTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    auto item1 = uploadInfos.appendItem();
    auto item2 = uploadInfos.appendItem();

    QCOMPARE(uploadInfos.getSelectedInfosCount(), 0);
    item1->setIsSelected(true);
    QCOMPARE(uploadInfos.getSelectedInfosCount(), 1);
    item2->setIsSelected(true);
    QCOMPARE(uploadInfos.getSelectedInfosCount(), 2);
}

void UploadInfoRepositoryTests::removeItemTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    auto item1 = uploadInfos.appendItem();
    auto item2 = uploadInfos.appendItem();

    QCOMPARE(uploadInfos.getInfosCount(), 2);

    uploadInfos.removeItem(0);

    QCOMPARE(uploadInfos.getInfosCount(), 1);
}

void UploadInfoRepositoryTests::missingDetailsTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    QVERIFY(uploadInfos.getAgenciesWithMissingDetails().isEmpty());

    auto item1 = uploadInfos.appendItem();
    item1->setTitle("my Title");
    item1->setHost("localhost");
    item1->setUsername("anonymous");

    QVERIFY(uploadInfos.getAgenciesWithMissingDetails().isEmpty());

    item1->setIsSelected(true);

    QCOMPARE(uploadInfos.getAgenciesWithMissingDetails(), QString("my Title"));

    item1->setPassword("q3rqefrqet2trewtq4y");

    QVERIFY(uploadInfos.getAgenciesWithMissingDetails().isEmpty());
}

void UploadInfoRepositoryTests::notFullUploadInfoTest() {
    Models::UploadInfo ui;

    QVERIFY(ui.isSomethingMissing());

    ui.setHost("localhost");
    QVERIFY(ui.isSomethingMissing());
    ui.setUsername("anonymous");
    QVERIFY(ui.isSomethingMissing());
    ui.setPassword("qwerajfldfa;a");

    QVERIFY(!ui.isSomethingMissing());
}

void UploadInfoRepositoryTests::findByHostTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    QVERIFY(uploadInfos.getAgenciesWithMissingDetails().isEmpty());

    auto item1 = uploadInfos.appendItem();
    item1->setTitle("UploadHost1");
    item1->setHost("localhost");

    auto item2 = uploadInfos.appendItem();
    item2->setTitle("Shutterstock");
    item2->setHost("shutterstock.com");

    QCOMPARE(uploadInfos.tryFindItemByHost("ftp://shutterstock.com/")->getHost(), item2->getHost());
    QVERIFY(uploadInfos.tryFindItemByHost("ftp://test.com/") == nullptr);
}

void UploadInfoRepositoryTests::setPasswordIsEncodedTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    uploadInfos.appendItem();
    auto index = uploadInfos.index(0);
    QString password = "qwerty";
    uploadInfos.setData(index,
                        QVariant::fromValue(QString("Shutterstock")),
                        Models::UploadInfoRepository::EditTitleRole);
    uploadInfos.setData(index,
                        QVariant::fromValue(password),
                        Models::UploadInfoRepository::EditPasswordRole);

    QVERIFY(uploadInfos.accessUploadInfos().front()->getPassword() != password);
    QVERIFY(uploadInfos.data(index, Models::UploadInfoRepository::PasswordRole).toString() == password);
}

void UploadInfoRepositoryTests::initAccountsWithCorrectMPTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    for (int i = 0; i < 3; i++) {
        uploadInfos.addItem();
        uploadInfos.setData(uploadInfos.index(i),
                            QVariant::fromValue(QString("qwerty %1").arg(i)),
                            Models::UploadInfoRepository::EditPasswordRole);
    }

    bool masterPasswordCorrectOrEmpty = true;
    uploadInfos.initializeAccounts(masterPasswordCorrectOrEmpty);

    for (int i = 0; i < 3; i++) {
        QCOMPARE(uploadInfos.data(uploadInfos.index(i), Models::UploadInfoRepository::PasswordRole).toString(),
                 QString("qwerty %1").arg(i));
    }
}

void UploadInfoRepositoryTests::initAccountsWithWrongMPTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    for (int i = 0; i < 3; i++) {
        uploadInfos.appendItem();
        uploadInfos.setData(uploadInfos.index(i),
                            QVariant::fromValue(QString("qwerty %1").arg(i)),
                            Models::UploadInfoRepository::EditPasswordRole);
    }

    bool masterPasswordCorrectOrEmpty = false;
    uploadInfos.initializeAccounts(masterPasswordCorrectOrEmpty);

    for (int i = 0; i < 3; i++) {
        QVERIFY(uploadInfos
                .data(uploadInfos.index(i), Models::UploadInfoRepository::PasswordRole)
                .toString()
                .isEmpty());
    }
}

void UploadInfoRepositoryTests::finalizeAccountsRestoresPasswordTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    for (int i = 0; i < 3; i++) {
        uploadInfos.addItem();
        uploadInfos.setData(uploadInfos.index(i),
                            QVariant::fromValue(QString("qwerty %1").arg(i)),
                            Models::UploadInfoRepository::EditPasswordRole);
    }

    bool masterPasswordCorrectOrEmpty = false;
    uploadInfos.initializeAccounts(masterPasswordCorrectOrEmpty);
    uploadInfos.finalizeAccounts();

    for (int i = 0; i < 3; i++) {
        QCOMPARE(uploadInfos.data(uploadInfos.index(i), Models::UploadInfoRepository::PasswordRole).toString(),
                 QString("qwerty %1").arg(i));
    }
}

void UploadInfoRepositoryTests::finalizeAccountsNoEffectTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    for (int i = 0; i < 3; i++) {
        uploadInfos.addItem();
        uploadInfos.setData(uploadInfos.index(i),
                            QVariant::fromValue(QString("qwerty %1").arg(i)),
                            Models::UploadInfoRepository::EditPasswordRole);
    }

    bool masterPasswordCorrectOrEmpty = true;
    uploadInfos.initializeAccounts(masterPasswordCorrectOrEmpty);
    uploadInfos.finalizeAccounts();

    for (int i = 0; i < 3; i++) {
        QCOMPARE(uploadInfos.data(uploadInfos.index(i), Models::UploadInfoRepository::PasswordRole).toString(),
                 QString("qwerty %1").arg(i));
    }
}

void UploadInfoRepositoryTests::defaultMasterPasswordChangeTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    QStringList oldEncodedPasswords;
    for (int i = 0; i < 3; i++) {
        auto item = uploadInfos.appendItem();
        uploadInfos.setData(uploadInfos.index(i),
                            QVariant::fromValue(QString("qwerty %1").arg(i)),
                            Models::UploadInfoRepository::EditPasswordRole);
        oldEncodedPasswords.append(item->getPassword());
    }

    bool firstTime = true;
    const QString specialMP = "secret master p";
    QVERIFY(secretsManager.changeMasterPassword(firstTime, QString(), specialMP));

    for (int i = 0; i < 3; i++) {
        QVERIFY(uploadInfos.accessUploadInfos().at(i)->getPassword() != oldEncodedPasswords[i]);
        QCOMPARE(uploadInfos.data(uploadInfos.index(i), Models::UploadInfoRepository::PasswordRole).toString(),
                 QString("qwerty %1").arg(i));
    }
}

void UploadInfoRepositoryTests::masterPasswordChangeTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    for (int i = 0; i < 3; i++) {
        uploadInfos.addItem();
        uploadInfos.setData(uploadInfos.index(i),
                            QVariant::fromValue(QString("qwerty %1").arg(i)),
                            Models::UploadInfoRepository::EditPasswordRole);
    }

    const QString specialMP1 = "secret master p";
    const QString specialMP2 = "another mp";
    QVERIFY(secretsManager.changeMasterPassword(true, QString(), specialMP1));

    QStringList oldEncodedPasswords;
    for (int i = 0; i < 3; i++) {
        oldEncodedPasswords.append(uploadInfos.accessUploadInfos().at(i)->getPassword());
    }

    QVERIFY(!secretsManager.changeMasterPassword(false, "incorrect mp", specialMP2));
    QVERIFY(secretsManager.changeMasterPassword(false, specialMP1, specialMP2));

    for (int i = 0; i < 3; i++) {
        QVERIFY(uploadInfos.accessUploadInfos().at(i)->getPassword() != oldEncodedPasswords[i]);
        QCOMPARE(uploadInfos.data(uploadInfos.index(i), Models::UploadInfoRepository::PasswordRole).toString(),
                 QString("qwerty %1").arg(i));
    }
}

void UploadInfoRepositoryTests::defaultMasterPasswordResetTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    QStringList oldEncodedPasswords;
    for (int i = 0; i < 3; i++) {
        auto item = uploadInfos.appendItem();
        uploadInfos.setData(uploadInfos.index(i),
                            QVariant::fromValue(QString("qwerty %1").arg(i)),
                            Models::UploadInfoRepository::EditPasswordRole);
        oldEncodedPasswords.append(item->getPassword());
    }

    secretsManager.resetMasterPassword();

    for (int i = 0; i < 3; i++) {
        QVERIFY(uploadInfos.accessUploadInfos().at(i)->getPassword() == oldEncodedPasswords[i]);
        QCOMPARE(uploadInfos.data(uploadInfos.index(i), Models::UploadInfoRepository::PasswordRole).toString(),
                 QString("qwerty %1").arg(i));
    }
}

void UploadInfoRepositoryTests::masterPasswordResetTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);

    QStringList oldEncodedPasswords;
    for (int i = 0; i < 3; i++) {
        auto item = uploadInfos.appendItem();
        uploadInfos.setData(uploadInfos.index(i),
                            QVariant::fromValue(QString("qwerty %1").arg(i)),
                            Models::UploadInfoRepository::EditPasswordRole);
        oldEncodedPasswords.append(item->getPassword());
    }

    QVERIFY(secretsManager.changeMasterPassword(true, QString(), QString("special MP")));

    for (int i = 0; i < 3; i++) {
        QVERIFY(uploadInfos.accessUploadInfos().at(i)->getPassword() != oldEncodedPasswords[i]);
        QVERIFY(uploadInfos.data(uploadInfos.index(i), Models::UploadInfoRepository::PasswordRole).toString() ==
                 QString("qwerty %1").arg(i));
    }

    secretsManager.resetMasterPassword();

    for (int i = 0; i < 3; i++) {
        QVERIFY(uploadInfos.accessUploadInfos().at(i)->getPassword() == oldEncodedPasswords[i]);
        QCOMPARE(uploadInfos.data(uploadInfos.index(i), Models::UploadInfoRepository::PasswordRole).toString(),
                 QString("qwerty %1").arg(i));
    }
}

void UploadInfoRepositoryTests::validateNonAutocompletedHostTest() {
    Mocks::CoreTestsEnvironment environment;
    Encryption::SecretsManager secretsManager;
    Models::UploadInfoRepository uploadInfos(environment, secretsManager);
    const char *stocksJson = R"JSON(
    {
        "overwrite": false,
        "stocks_ftp": [
            {
                "name": "Alamy",
                "ftp": "ftp://upload.alamy.com",
                "images_dir": "Stock",
                "vectors_dir": "Vector"
            }
        ]
    }
)JSON";

    QByteArray jsonData(stocksJson);
    uploadInfos.accessStocksList().setRemoteOverride(jsonData);

    Helpers::AsyncCoordinator coordinator;
    Mocks::RequestsServiceMock requestsService;
    uploadInfos.initializeStocksList(coordinator, requestsService);

    auto item1 = uploadInfos.appendItem();
    uploadInfos.setCurrentIndex(0);
    item1->setTitle("alamy");

    QCOMPARE(item1->getImagesDir(), QString());
    QCOMPARE(item1->getVectorsDir(), QString());

    uploadInfos.validateCurrentItem();

    QCOMPARE(item1->getImagesDir(), QString("Stock"));
    QCOMPARE(item1->getVectorsDir(), QString("Vector"));
}
