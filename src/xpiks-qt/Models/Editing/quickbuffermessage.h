/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QUICKBUFFERMESSAGE_H
#define QUICKBUFFERMESSAGE_H

#include <QString>
#include <QStringList>

namespace Models {
    struct QuickBufferMessage {
        QuickBufferMessage(const QString &title,
                           const QString &description,
                           const QStringList &keywords,
                           bool override = false):
            m_Title(title),
            m_Description(description),
            m_Keywords(keywords),
            m_Override(override)
        {
        }

        QString m_Title;
        QString m_Description;
        QStringList m_Keywords;
        bool m_Override = false;
    };
}

#endif // QUICKBUFFERMESSAGE_H
