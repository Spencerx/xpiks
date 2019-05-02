/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ANALYTICSUSEREVENT
#define ANALYTICSUSEREVENT

#include <QDateTime>
#include <QLatin1String>
#include <QString>
#include <QTime>

namespace Connectivity {
    enum struct EventType {
        Open,
        Close,
        Upload,
        SpellCheck,
        SpellSuggestions,
        SuggestionRemote,
        SuggestionLocal,
        WarningsCheck,
        TurnOffTelemetry,
        FindAndReplace,
        DeleteKeywords,
        SuggestionCancel,
        SetupFindAndReplace,
        FindReplaceCandidates,
        SetupDeleteKeywords,
        CheckUploadCredentials,
        AddFiles,
        AddDirectories,
        DropFilesDirectories
    };

    class AnalyticsUserEvent {
    public:
        AnalyticsUserEvent(EventType action) :
            m_Action(action)
        {
            m_DateTime = QDateTime::currentDateTimeUtc();
        }

        QString getActionString() const {
            switch (m_Action) {
                case EventType::Open: return QLatin1String("Open");
                case EventType::Close: return QLatin1String("Close");
                case EventType::Upload: return QLatin1String("Upload");
                case EventType::SpellCheck: return QLatin1String("SpellCheck");
                case EventType::SpellSuggestions: return QLatin1String("SpellSuggestions");
                case EventType::SuggestionRemote: return QLatin1String("SuggestionRemote");
                case EventType::SuggestionLocal: return QLatin1String("SuggestionLocal");
                case EventType::WarningsCheck: return QLatin1String("WarningsCheck");
                case EventType::TurnOffTelemetry: return QLatin1String("TurnOffTelemetry");
                case EventType::FindAndReplace: return QLatin1String("UserActionFindAndReplace");
                case EventType::DeleteKeywords: return QLatin1String("UserActionDeleteKeywords");
                case EventType::SuggestionCancel: return QLatin1String("SuggestionCancel");
                case EventType::SetupFindAndReplace: return QLatin1String("SetupFindAndReplace");
                case EventType::FindReplaceCandidates: return QLatin1String("FindReplaceCandidates");
                case EventType::SetupDeleteKeywords: return QLatin1String("SetupDeleteKeywords");
                case EventType::CheckUploadCredentials: return QLatin1String("CheckUploadCredentials");
                case EventType::AddFiles: return QLatin1String("AddFiles");
                case EventType::AddDirectories: return QLatin1String("AddDirectories");
                case EventType::DropFilesDirectories: return QLatin1String("DropFilesDirectories");
                default: return QLatin1String("Unknown");
            }
        }

        int getHour() const {
            return m_DateTime.time().hour();
        }

        int getMinute() const {
            return m_DateTime.time().minute();
        }

        int getSecond() const {
            return m_DateTime.time().second();
        }

    private:
        EventType m_Action;
        QDateTime m_DateTime;
    };

}

#endif // ANALYTICSUSEREVENT

