#include "encryption_tests.h"

#include <string>

#include <QTime>
#include <QtGlobal>

#include "Encryption/aes-qt.h"
#include "Encryption/obfuscation.h"

#include "stringhelpersfortests.h"

void EncryptionTests::simpleEncodeDecodeTest() {
    QString text = QString::fromUtf8("simple text");
    QString key = QString::fromUtf8("key");
    QString encoded = Encryption::encodeText(text, key);
    QString decoded = Encryption::decodeText(encoded, key);

    QCOMPARE(decoded, text);
}

void EncryptionTests::simpleDecodeEncodeTest() {
    QString key2 = QString::fromUtf8("DefaultMasterPassword");
    QString text = QString::fromUtf8("simple text");
    QString key1 = QString::fromUtf8("DefaultMasterPassword");
    QString encoded1 = Encryption::encodeText(text, key1);
    QString decoded = Encryption::decodeText(encoded1, key1);
    QString encoded2 = Encryption::encodeText(decoded, key2);
    QCOMPARE(encoded2, encoded1);
}

void EncryptionTests::encodeDecodeEncodeTest() {
#if defined(TRAVIS_CI) || defined(APPVEYOR)
    int iterations = 10000;
#else
    int iterations = 100;
#endif
    for (int i = 0; i < iterations; i++) {
        QString key2 = QString("DefaultMasterPassword %1").arg(i % 2 == 0 ? i : 1);
        QString text = getRandomString(qrand() % 100);
        QString key1 = QString("DefaultMasterPassword %1").arg((i & 1) == 0 ? i : 1);
        QString encoded1 = Encryption::encodeText(text, key1);
        QString decoded = Encryption::decodeText(encoded1, key1);
        QString encoded2 = Encryption::encodeText(decoded, key2);

        if (encoded2 != encoded1) {
            QFAIL(QString("Text: %1\encoded 1: %2\nencoded 2: %3").arg(text).arg(encoded1).arg(encoded2)
                  .toStdString().c_str());
            break;
        }
    }
}

void EncryptionTests::encodedTextNotEqualToRawTest() {
    QString text = "yet another simple text";
    QString key = "some more sophisticated key q4234";
    QString encoded = Encryption::encodeText(text, key);

    QVERIFY(encoded != text);
}

void EncryptionTests::encodedNotEqualToDecodedTest() {
    QString text = "yet another simple text";
    QString key = "some more sophisticated key q4234";
    QString encoded = Encryption::encodeText(text, key);
    QString decoded = Encryption::decodeText(encoded, key);

    QVERIFY(encoded != decoded);
}

void EncryptionTests::complexEncodeDecodeTest() {
    QString text = "simple text which has end of line and consists of \n newlines and more \t symbols";
    QString key = "key with \t tabulation and whitespaces and";
    QString encoded = Encryption::encodeText(text, key);
    QString decoded = Encryption::decodeText(encoded, key);

    QCOMPARE(decoded, text);
}

void EncryptionTests::simpleEncodeDecodeCyrrylicTest() {
    QString text = QString::fromUtf8("простий текст");
    QString key =  QString::fromUtf8("ключ");
    QString encoded = Encryption::encodeText(text, key);
    QString decoded = Encryption::decodeText(encoded, key);

    QCOMPARE(decoded, text);
}

void EncryptionTests::simpleEncodeDecodeUtf8Test() {
    QString text = QString::fromUtf8("Test \u20AC");;
    QString key = QString::fromUtf8("Test \u20AC\u20AC");;
    QString encoded = Encryption::encodeText(text, key);
    QString decoded = Encryption::decodeText(encoded, key);

    QCOMPARE(decoded, text);
}

void EncryptionTests::realTest() {
    QString text = "mp";
    QString key = "93b7294e86d8d9923a497364fc8148befd67f46a";
    QString encoded = Encryption::encodeText(text, key);
    QString decoded = Encryption::decodeText(encoded, key);

    QCOMPARE(decoded, text);
}

void EncryptionTests::bigRandomTest() {
#if defined(TRAVIS_CI) || defined(APPVEYOR)
    int iterations = 10000;
#else
    int iterations = 500;
#endif
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    while (iterations--) {
        QString key = getRandomString(qrand() % 100);
        QString randomString = getRandomString(qrand() % 1000);
        QString encoded = Encryption::encodeText(randomString, key);
        QString decoded = Encryption::decodeText(encoded, key);
        if (randomString != decoded) {
            QFAIL(QString("Text: %1\nDecoded: %2\nKey: %3").arg(randomString).arg(decoded).arg(key)
                  .toStdString().c_str());
            break;
        }
    }
}

void EncryptionTests::bigRandomTestForSmallValues() {
#if defined(TRAVIS_CI) || defined(APPVEYOR)
    int iterations = 10000;
#else
    int iterations = 100;
#endif
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    while (iterations--) {
        QString key = getRandomString(qrand() % 10);
        QString randomString = getRandomString(qrand() % 10);
        QString encoded = Encryption::encodeText(randomString, key);
        QString decoded = Encryption::decodeText(encoded, key);
        if (randomString != decoded) {
            QFAIL(QString("Text: %1\nDecoded: %2\nKey: %3").arg(randomString).arg(decoded).arg(key)
                  .toStdString().c_str());
            break;
        }
    }
}

void EncryptionTests::rot13basicTest() {
    int iterations = 10000;

    while (iterations--) {
        QString randomString = getRandomString(qrand() % 1000);
        QCOMPARE(Encryption::rot13plus(Encryption::rot13plus(randomString)), randomString);
    }

    iterations = 15000;

    while (iterations--) {
        QString randomString = getRandomByteString(qrand() % 1000);
        QCOMPARE(Encryption::rot13plus(Encryption::rot13plus(randomString)), randomString);
    }
}
