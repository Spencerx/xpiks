#ifndef RESTARTMODEL_H
#define RESTARTMODEL_H

#include <QObject>
#include <QStringList>

class RestartModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString arguments READ getArguments CONSTANT)
public:
    explicit RestartModel(const QStringList &arguments, QObject *parent = 0);

public:
    Q_INVOKABLE void recover();

public:
    QString getArguments() const { return m_Arguments.join(" "); }

private:
    QStringList m_Arguments;
};

#endif // RESTARTMODEL_H
