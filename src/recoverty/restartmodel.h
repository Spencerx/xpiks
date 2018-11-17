#ifndef RESTARTMODEL_H
#define RESTARTMODEL_H

#include <QObject>
#include <QString>
#include <QStringList>

class RestartModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString arguments READ getArguments CONSTANT)
    Q_PROPERTY(QString appName READ getAppName CONSTANT)
public:
    explicit RestartModel(const QStringList &arguments, QObject *parent = nullptr);

public:
    Q_INVOKABLE void recover();

public:
    QString getArguments() const { return m_Arguments.join(" "); }
    const QString &getAppName() const { return m_AppName; }

private:
    QString m_AppName;
    QStringList m_Arguments;
};

#endif // RESTARTMODEL_H
