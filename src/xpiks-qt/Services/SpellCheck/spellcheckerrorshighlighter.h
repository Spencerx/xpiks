/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SPELLCHECKERRORSHIGHLIGHTER_H
#define SPELLCHECKERRORSHIGHLIGHTER_H

#include <QObject>
#include <QString>
#include <QSyntaxHighlighter>

class QTextDocument;

namespace QMLExtensions {
    class ColorsModel;
}

namespace SpellCheck {
    class SpellCheckErrorsInfo;

    class SpellCheckErrorsHighlighter : public QSyntaxHighlighter
    {
        Q_OBJECT
    public:
        SpellCheckErrorsHighlighter(QTextDocument *document,
                                    QMLExtensions::ColorsModel *colorsModel,
                                    SpellCheckErrorsInfo *errorsInfo);
        virtual ~SpellCheckErrorsHighlighter();

    protected:
        virtual void highlightBlock(const QString &text) override;

    private:
        SpellCheckErrorsInfo *m_SpellCheckErrors;
        QMLExtensions::ColorsModel *m_ColorsModel;
    };
}

#endif // SPELLCHECKERRORSHIGHLIGHTER_H
