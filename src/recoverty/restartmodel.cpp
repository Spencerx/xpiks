#include "restartmodel.h"
#include <QProcess>

RestartModel::RestartModel(const QStringList &arguments, QObject *parent) :
    QObject(parent),
    m_AppName("Xpiks"),
    m_Arguments(arguments)
{
}

void RestartModel::recover() {
    if (m_Arguments.size() < 1) { return; }
    QProcess::startDetached(m_Arguments.first(), m_Arguments.mid(1));
}
