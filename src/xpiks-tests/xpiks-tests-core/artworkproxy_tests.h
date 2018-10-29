#ifndef ARTWORKPROXY_TESTS_H
#define ARTWORKPROXY_TESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

class ArtworkProxyTests: public QObject
{
    Q_OBJECT
private slots:
    void editKeywordsPropagatesToArtworkTest();
    void editTitlePropagatesToArtworkTest();
    void editDescriptionPropagatesToArtworkTest();
};

#endif // ARTWORKPROXY_TESTS_H
