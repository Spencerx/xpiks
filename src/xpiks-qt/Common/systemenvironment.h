/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SYSTEMENVIRONMENT_H
#define SYSTEMENVIRONMENT_H

#include <QString>
#include <QStringList>

#include "Common/isystemenvironment.h"

namespace Common {
    class SystemEnvironment: public ISystemEnvironment
    {
    public:
        SystemEnvironment(const QStringList &appArguments = QStringList());

    public:
        virtual void ensureSystemDirectoriesExist();

    public:
        virtual QString root() const override { return m_Root; }
        virtual QString path(const QStringList &path) override;
        virtual bool ensureDirExists(const QString &name) override;
        virtual bool getIsInMemoryOnly() const override { return m_InMemoryExperiment; }
        virtual bool getIsRecoveryMode() const override { return m_IsRecoveryMode; }

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
    protected:
        const QString &getSessionTag() const { return m_SessionTag; }
#endif

    private:
        QString m_Root;
        QString m_SessionTag;
        bool m_InMemoryExperiment = false;
        bool m_IsRecoveryMode = false;
    };
}

#endif // SYSTEMENVIRONMENT_H
