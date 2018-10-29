#ifndef RC6_TESTS
#define RC6_TESTS

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class EncryptionTests : public QObject
{
    Q_OBJECT
private slots:
    void simpleEncodeDecodeTest();
    void encodedTextNotEqualToRawTest();
    void encodedNotEqualToDecodedTest();
    void complexEncodeDecodeTest();
    void simpleEncodeDecodeCyrrylicTest();
    void simpleEncodeDecodeUtf8Test();
    void realTest();
    void bigRandomTest();
    void bigRandomTestForSmallValues();
    void rot13basicTest();
};

#endif // RC6_TESTS

