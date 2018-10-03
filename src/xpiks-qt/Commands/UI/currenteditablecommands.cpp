/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "currenteditablecommands.h"
#include <Commands/Base/icommandmanager.h>
#include <Common/logging.h>
#include <Models/Editing/icurrenteditable.h>
#include <Helpers/stringhelper.h>

namespace Commands {
    namespace UI {
        void PlainTextEditCommand::execute(const QVariant &value) {
            LOG_DEBUG << "#";
            auto currentEditable = m_CurrentEditableSource.getCurrentEditable();
            if (currentEditable != nullptr) {
                QString rawKeywords;
                bool spaceIsSeparator = false;
                if (value.isValid()) {
                    auto map = value.toMap();
                    auto textValue = map.value("text", QVariant(""));
                    if (textValue.type() == QVariant::String) {
                        rawKeywords = textValue.toString();
                    }
                    auto spaceIsSeparatorValue = map.value("spaceIsSeparator", QVariant(false));
                    if (spaceIsSeparatorValue.type() == QVariant::Bool) {
                        spaceIsSeparator = spaceIsSeparatorValue.toBool();
                    }
                }

                QVector<QChar> separators;
                separators << QChar(',');
                if (spaceIsSeparator) { separators << QChar::Space; }
                QStringList keywords;
                Helpers::splitKeywords(rawKeywords.trimmed(), separators, keywords);

                auto command = currentEditable->setKeywords(keywords);
                m_CommandManager.processCommand(command);
            }
        }

        void AppendSuggestedKeywordsCommand::execute(const QVariant &value) {
            LOG_DEBUG << "#";
            auto currentEditable = m_CurrentEditableSource.getCurrentEditable();
            if (currentEditable != nullptr) {
                if (value.type() == QVariant::StringList) {
                    QStringList keywords = value.toStringList();
                    auto command = currentEditable->appendKeywords(keywords);
                    m_CommandManager.processCommand(command);
                } else {
                    LOG_WARNING << "Argument's type is non-compatible:" << value.typeName();
                }
            }
        }
    }
}
