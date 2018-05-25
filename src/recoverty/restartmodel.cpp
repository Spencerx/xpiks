#include "restartmodel.h"
#include <QProcess>

RestartModel::RestartModel(const QStringList &arguments, QObject *parent) :
    QObject(parent),
    m_Arguments(arguments)
{
}

void RestartModel::recover() {
    if (m_Arguments.isEmpty()) { return; }
    QProcess::startDetached(m_Arguments.first(), m_Arguments.mid(1));
}
