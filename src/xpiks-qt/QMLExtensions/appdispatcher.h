/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef APPDISPATCHER_H
#define APPDISPATCHER_H

#include <QJSValue>
#include <QObject>
#include <unordered_map>
#include "iuicommanddispatcher.h"

namespace Commands {
    class ICommandManager;
}

namespace QMLExtensions {
    class AppDispatcher : public QObject
    {
        Q_OBJECT
    public:
        explicit AppDispatcher(QObject *parent = 0);

    public:
        Q_INVOKABLE void dispatch(int commandID, QJSValue value = QJSValue());

    public:
        virtual void registerCommand(int commandID, const std::shared_ptr<Commands::IUICommandTemplate> &command) override;

    private:
        std::unordered_multimap<int, std::shared_ptr<Commands::IUICommandTemplate>> m_ComandsMap;
        Commands::ICommandManager &m_CommandManager;
    };
}

#endif // APPDISPATCHER_H
