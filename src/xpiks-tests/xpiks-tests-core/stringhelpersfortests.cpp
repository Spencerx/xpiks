#include "stringhelpersfortests.h"

#include <QChar>
#include <QtGlobal>

QString getRandomString(int length, bool exactSize) {
    QString chars("abcdefghijklmnopqrstuvwxyz      ABCDEFGHIJKLMNOPQRSTUVWXYZ    0123456789   /!@ #$% ^&*()_+ =|");
    const int size = length + (exactSize ? 0 : (qrand() % 20));
    return getRandomString(chars, size);
}

QString getRandomString(const QString &alphabet, int length) {
    if (length <= 0) { return QString(); }

    QString result;
    result.reserve(length);
    const int charsLength = alphabet.size() - 1;

    while (length--) {
        result.append(alphabet[qrand() % charsLength]);
    }

    return result;
}

QString getRandomByteString(int length) {
    QString result;
    result.reserve(length);
    while (length--) {
        result.append(QChar(ushort(qrand() % 0xffff)));
    }
    return result;
}
