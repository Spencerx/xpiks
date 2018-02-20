#ifndef CORETESTSENVIRONMENT_H
#define CORETESTSENVIRONMENT_H

#include "../../xpiks-qt/Common/isystemenvironment.h"

namespace Mocks {
    class CoreTestsEnvironment: public Common::ISystemEnvironment {
        virtual QString root() const override { return QString(); }
        virtual QString fileInDir(const QString &filename, const QString &dirname) override { return filename; }
        virtual QString filepath(const QString &name) override { return name; }
        virtual QString dirpath(const QString &name) override { return name; }
        virtual void ensureDirExists(const QString &name) override { }
    };
}

#endif // CORETESTSENVIRONMENT_H
