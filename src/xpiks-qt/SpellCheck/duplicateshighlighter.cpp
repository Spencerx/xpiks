/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "duplicateshighlighter.h"
#include <QColor>
#include <QSet>
#include <QString>
#include "spellcheckiteminfo.h"
#include "../QMLExtensions/colorsmodel.h"
#include "../Helpers/stringhelper.h"
#include "../Common/defines.h"
#include "../Common/basickeywordsmodel.h"

namespace SpellCheck {
    DuplicatesHighlighter::DuplicatesHighlighter(QTextDocument *document,
                                                 QMLExtensions::ColorsModel *colorsModel,
                                                 SpellCheckErrorsInfo *errorsInfo,
                                                 bool highlightAll) :
        QSyntaxHighlighter(document),
        m_SpellCheckErrors(errorsInfo),
        m_ColorsModel(colorsModel),
        m_HighlighAll(highlightAll)
    {
        Q_ASSERT(colorsModel != nullptr);
    }

    DuplicatesHighlighter::~DuplicatesHighlighter() {
        // LOG_FOR_DEBUG << "destroyed";
    }

    void DuplicatesHighlighter::highlightBlock(const QString &text) {
        if ((m_SpellCheckErrors != nullptr) && !m_SpellCheckErrors->anyDuplicate()) { return; }

        QColor modifiedColor = m_ColorsModel->artworkModifiedColor();
        QColor darkColor = m_ColorsModel->defaultDarkColor();
        QTextCharFormat duplicatesFormat;
        duplicatesFormat.setBackground(QBrush(modifiedColor));
        duplicatesFormat.setForeground(QBrush(darkColor));

        const QString textLower = text.toLower();

        Helpers::foreachWord(textLower,
                             [this](const QString &word) {
            return m_SpellCheckErrors != nullptr ?
                        this->m_SpellCheckErrors->hasDuplicates(word) :
                        m_HighlighAll;
        },
        [this, &duplicatesFormat](int start, int length, const QString &) {
            this->setFormat(start, length, duplicatesFormat);
        });
    }

    void DuplicatesHighlighter::keywordsDuplicatesChanged() {
        Common::BasicKeywordsModel *basicModel = qobject_cast<Common::BasicKeywordsModel *>(sender());
        Q_ASSERT(basicModel != nullptr);
        if (basicModel != nullptr) {
            m_HighlighAll = basicModel->hasKeywordsDuplicates();
        }

        rehighlight();
    }
}

