#include "restartmodel.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#include <string>
#include <vector>
#endif

#include <QString>
#include <QtGlobal>

#ifndef Q_OS_LINUX
    #include <QProcess>
#endif

RestartModel::RestartModel(const QStringList &arguments, QObject *parent) :
    QObject(parent),
    m_AppName("Xpiks"),
    m_Arguments(arguments)
{
}

void RestartModel::recover() {
    if (m_Arguments.size() < 1) { return; }
#ifndef Q_OS_LINUX
    QProcess::startDetached(m_Arguments.first(), m_Arguments.mid(1));
#else
    std::string path = m_Arguments.first().toStdString();
    std::vector<std::string> arguments;
    for (auto &a: m_Arguments) {
        arguments.emplace_back(a.toStdString());
    }

    const char **argv = new const char *[m_Arguments.size() + 1];
    int i = 0;
    for (auto &a: arguments) {
        argv[i++] = a.c_str();
    }
    argv[i] = (char*)nullptr;
    execv(path.c_str(), (char * const *)argv);
    //execl(path.c_str(), path.c_str(), (char*)nullptr);
#endif
}
