/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "loghighlighter.h"
#include <QColor>
#include <QtGlobal>
#include "../QMLExtensions/colorsmodel.h"

namespace Helpers {
    LogHighlighter::LogHighlighter(QMLExtensions::ColorsModel &colorsModel, QTextDocument *document) :
        QSyntaxHighlighter(document),
        m_ColorsModel(colorsModel)
    {
    }

    void LogHighlighter::highlightBlock(const QString &text) {
        int size = text.size();

        QColor destructiveColor = m_ColorsModel.destructiveColor();
        QColor artworkModifiedColor = m_ColorsModel.artworkModifiedColor();
        QColor labelActiveForeground = m_ColorsModel.labelActiveForeground();

        QString word = text.mid(13, 13+8).toLower();

        if (word.startsWith(QLatin1Literal("debug"))) {
            // for Qt < 5.5.1 "info" msgtype does not exist
            // so we will use default color for debug in old Qt
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 1))
            setFormat(0, size, labelActiveForeground);
#endif
        } else if (word.startsWith(QLatin1Literal("info"))) {
            // DO NOTHING - USE DEFAULT COLOR
        } else if (word.startsWith(QLatin1Literal("warning"))) {
            setFormat(0, size, artworkModifiedColor);
        } else if (word.startsWith(QLatin1Literal("critical"))) {
            setFormat(0, size, destructiveColor);
        } else if (word.startsWith(QLatin1Literal("fatal"))) {
            setFormat(0, size, destructiveColor);
        }
    }
}
