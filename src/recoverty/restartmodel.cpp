#include "restartmodel.h"
#include <QProcess>

RestartModel::RestartModel(const QStringList &arguments, QObject *parent) :
    QObject(parent),
    m_AppName("App"),
    m_Arguments(arguments)
{
    if (!m_Arguments.empty()) {
        m_AppName = m_Arguments.first();
    }
}

void RestartModel::recover() {
    if (m_Arguments.size() < 2) { return; }
    QProcess::startDetached(m_Arguments[1], m_Arguments.mid(2));
}
