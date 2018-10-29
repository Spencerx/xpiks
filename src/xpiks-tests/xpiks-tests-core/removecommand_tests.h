#ifndef REMOVECOMMAND_TESTS_H
#define REMOVECOMMAND_TESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class RemoveCommandTests : public QObject
{
    Q_OBJECT

private slots:
    void removeArtworksFromEmptyRepository();
    void removeAllArtworksFromRepository();
};

#endif // REMOVECOMMAND_TESTS_H
