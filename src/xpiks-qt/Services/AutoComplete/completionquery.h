/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMPLETIONQUERY_H
#define COMPLETIONQUERY_H

#include <vector>

#include <QObject>
#include <QString>

#include "Common/flags.h"
#include "Common/types.h"

namespace AutoComplete {
    struct CompletionResult {
        CompletionResult(const QString &completion):
            m_Completion(completion),
            m_PresetID(-1)
        {
        }

        CompletionResult(const QString &completion, int presetID):
            m_Completion(completion),
            m_PresetID(presetID)
        {
        }

        QString m_Completion;
        int m_PresetID;
    };

    class CompletionQuery: public QObject
    {
        Q_OBJECT
    public:
        CompletionQuery(const QString &prefix):
            m_Prefix(prefix),
            m_CompletionFlags(0)
        {
        }

    private:
        enum CompletionQueryFlags {
            RequiresUpdateFlag = 1 << 0,
            CompleteKeywordsFlag = 1 << 1,
            CompletePresetsFlag = 1 << 2
        };

        inline bool getRequiresUpdateFlag() const { return Common::HasFlag(m_CompletionFlags, RequiresUpdateFlag); }
        inline bool getCompleteKeywordsFlag() const { return Common::HasFlag(m_CompletionFlags, CompleteKeywordsFlag); }
        inline bool getCompletePresetsFlag() const { return Common::HasFlag(m_CompletionFlags, CompletePresetsFlag); }

        void setRequiresUpdateFlag(bool value) { Common::ApplyFlag(m_CompletionFlags, value, RequiresUpdateFlag); }
        void setCompleteKeywordsFlag(bool value) { Common::ApplyFlag(m_CompletionFlags, value, CompleteKeywordsFlag); }
        void setCompletePresetsFlag(bool value) { Common::ApplyFlag(m_CompletionFlags, value, CompletePresetsFlag); }

    public:
        const QString &getPrefix() const { return m_Prefix; }
        bool getNeedsUpdate() const { return getRequiresUpdateFlag(); }
        bool getCompleteKeywords() const { return getCompleteKeywordsFlag(); }
        bool getCompletePresets() const { return getCompletePresetsFlag(); }
        std::vector<CompletionResult> &getCompletions() { return m_Completions; }

    public:
        void setNeedsUpdate() { setRequiresUpdateFlag(true); }
        void setCompleteKeywords(bool value) { setCompleteKeywordsFlag(value); }
        void setCompletePresets(bool value) { setCompletePresetsFlag(value); }
        void setCompletions(std::vector<CompletionResult> &completions) { m_Completions.swap(completions); }
        void notifyCompletionsAvailable() { emit completionsAvailable(); }

    signals:
        void completionsAvailable();

    private:
        QString m_Prefix;
        std::vector<CompletionResult> m_Completions;
        // QString m_Context;
        volatile Common::flag_t m_CompletionFlags;
    };
}

#endif // COMPLETIONQUERY_H
