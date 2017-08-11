/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMPLETIONQUERY_H
#define COMPLETIONQUERY_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "autocompletemodel.h"
#include "../Common/defines.h"

namespace AutoComplete {
    class CompletionQuery : public QObject {
        Q_OBJECT
    public:
        CompletionQuery(const QString &prefix, AutoCompleteModel *autoCompleteModel) :
            m_Prefix(prefix),
            m_AutoCompleteModel(autoCompleteModel)
        {
            Q_ASSERT(autoCompleteModel != nullptr);
        }

        const QString &getPrefix() const { return m_Prefix; }

        void setCompletions(const QStringList &completions) {
            m_AutoCompleteModel->setCompletions(completions);
            emit completionsAvailable();
        }

    signals:
        void completionsAvailable();

    private:
        QString m_Prefix;
        AutoCompleteModel *m_AutoCompleteModel;
    };
}

#endif // COMPLETIONQUERY_H
