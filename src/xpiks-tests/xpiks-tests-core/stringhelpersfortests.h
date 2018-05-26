#ifndef STRINGHELPERSFORTESTS_H
#define STRINGHELPERSFORTESTS_H

#include <QString>

QString getRandomString(int length, bool exactSize=false);
QString getRandomString(const QString &alphabet, int length);
QString getRandomByteString(int length);

#endif // STRINGHELPERSFORTESTS_H
