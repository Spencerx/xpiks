#ifndef STRINGHELPERSTESTS_H
#define STRINGHELPERSTESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class StringHelpersTests : public QObject
{
    Q_OBJECT
private slots:
    void splitEmptyStringTest();
    void splitStringWithSpacesTest();
    void splitOneWordTest();
    void splitOneWordWithSpacesTest();
    void splitSeveralWordsWithSpacesTest();
    void splitOneLetterWordTest();
    void splitSeveralWordsWithPunctuationTest();
    void replaceWholeSmokeTest();
    void replaceWholeTrivialTest();
    void replaceWholeTrivialCaseSensitiveTest();
    void replaceWholePrefixTest();
    void replaceWholeSuffixTest();
    void replaceWholeWithPunctuationTest();
    void replaceWholeWithSpaceTest();
    void replaceWholeWithCommaTest();
    void replaceWholeNoCaseHitTest();
    void replaceWholeNoHitTest();
    void switcherHashTest();
    void basicUrlDecodeTest();
    void csvQuotesTest();
};

#endif // STRINGHELPERSTESTS_H
