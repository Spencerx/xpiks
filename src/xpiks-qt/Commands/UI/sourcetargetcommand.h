/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SOURCETARGETCOMMAND_H
#define SOURCETARGETCOMMAND_H

class QObject;

#include "Commands/Base/iuicommandtemplate.h"  // IWYU pragma: keep
#include "Services/SpellCheck/ispellcheckservice.h"  // IWYU pragma: keep
#include "Services/SpellCheck/spellchecksuggestionmodel.h"  // IWYU pragma: keep
#include "Services/iartworksupdater.h"  // IWYU pragma: keep

namespace Commands {
    namespace UI {
        class IUICommandTargetSource {
        public:
            virtual ~IUICommandTargetSource(){}
            virtual QObject *getTargetObject() = 0;
        };

#define SOURCE_COMMAND(ClassName, CommandID, Source) \
    class ClassName: public IUICommandTemplate {\
    public:\
        ClassName(Source &source):\
            m_Source(source)\
        {}\
    public:\
        virtual int getCommandID() override { return CommandID; }\
        virtual void execute(QVariant const &value) override;\
    private:\
        Source &m_Source;\
    };

#define TARGET_COMMAND(ClassName, CommandID, Target) \
    class ClassName: public IUICommandTemplate, public IUICommandTargetSource {\
    public:\
        ClassName(Target &target):\
            m_Target(target)\
        {}\
    public:\
        virtual int getCommandID() override { return CommandID; }\
        virtual void execute(QVariant const &value) override;\
        virtual QObject *getTargetObject() override { return (QObject*)&m_Target; }\
    private:\
        Target &m_Target;\
    };

#define SOURCE_TARGET_COMMAND(ClassName, CommandID, Source, Target) \
        class ClassName: public IUICommandTemplate {\
        public:\
            ClassName(Source &source, Target &target):\
                m_Source(source),\
                m_Target(target)\
            {}\
        public:\
            virtual int getCommandID() override { return CommandID; }\
            virtual void execute(QVariant const &value) override;\
        private:\
            Source &m_Source;\
            Target &m_Target;\
        };

        // same as SOURCE_TARGET_COMMAND but adds IUICommandTargetSource
        // interface so dispatcher could return command target for view
#define SOURCE_UI_TARGET_COMMAND(ClassName, CommandID, Source, Target) \
        class ClassName: public IUICommandTemplate, public IUICommandTargetSource {\
        public:\
            ClassName(Source &source, Target &target):\
                m_Source(source),\
                m_Target(target)\
            {}\
        public:\
            virtual int getCommandID() override { return CommandID; }\
            virtual void execute(QVariant const &value) override;\
            virtual QObject *getTargetObject() override { return (QObject*)&m_Target; }\
        private:\
            Source &m_Source;\
            Target &m_Target;\
        };

#define FIX_ARTWORK_SPELLING_COMMAND(ClassName, CommandID, Source)\
    class ClassName: public IUICommandTemplate, public IUICommandTargetSource {\
    public:\
        ClassName(Source &source,\
                  Services::IArtworksUpdater &artworksUpdater,\
                  SpellCheck::ISpellCheckService &spellCheckService,\
                  SpellCheck::SpellCheckSuggestionModel &spellSuggestionsModel):\
            m_Source(source),\
            m_ArtworksUpdater(artworksUpdater),\
            m_SpellCheckService(spellCheckService),\
            m_SpellSuggestionsModel(spellSuggestionsModel)\
        {}\
    public:\
        virtual int getCommandID() override { return CommandID; }\
        virtual void execute(const QVariant &value) override;\
        virtual QObject *getTargetObject() override { return (QObject*)&m_SpellSuggestionsModel; }\
    private:\
        Source &m_Source;\
        Services::IArtworksUpdater &m_ArtworksUpdater;\
        SpellCheck::ISpellCheckService &m_SpellCheckService;\
        SpellCheck::SpellCheckSuggestionModel &m_SpellSuggestionsModel;\
    };
    }
}

#endif // SOURCETARGETCOMMAND_H
