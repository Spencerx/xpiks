#ifndef TESTSHOST_H
#define TESTSHOST_H

#include <QObject>
#include <QMLExtensions/colorsmodel.h>

class QQmlEngine;

class TestsHost : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString scoreme READ getName CONSTANT)

public:
    static TestsHost& getInstance()
    {
        static TestsHost instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

public:
    void qmlEngineCallback(QQmlEngine *engine);

public:
    QString getName() const { return "testsHost"; }

private:
    explicit TestsHost(QObject *parent = 0);

    TestsHost(TestsHost const&);
    void operator=(TestsHost const&);

signals:

public slots:

private:
    QMLExtensions::ColorsModel m_ColorsModel;
};

#endif // TESTSHOST_H
