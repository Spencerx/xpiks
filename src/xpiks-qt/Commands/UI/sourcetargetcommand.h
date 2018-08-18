/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SOURCETARGETCOMMAND_H
#define SOURCETARGETCOMMAND_H

namespace Commands {
    namespace UI {
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

#define FIX_ARTWORK_SPELLING_COMMAND(ClassName, CommandID, Source)\
    class ClassName: public IUICommandTemplate {\
    public:\
        ClassName(Source &source,\
                  Services::IArtworksUpdater &artworksUpdater,\
                  SpellCheck::IArtworkSpellCheckService &spellCheckService,\
                  SpellCheck::SpellCheckSuggestionModel &spellSuggestionsModel):\
            m_Source(source),\
            m_ArtworksUpdater(artworksUpdater),\
            m_SpellCheckService(spellCheckService),\
            m_SpellSuggestionsModel(spellSuggestionsModel)\
        {}\
    public:\
        virtual int getCommandID() override { return CommandID; }\
        virtual void execute(const QVariant &value) override;\
    private:\
        Source &m_Source;\
        Services::IArtworksUpdater &m_ArtworksUpdater;\
        SpellCheck::IArtworkSpellCheckService &m_SpellCheckService;\
        SpellCheck::SpellCheckSuggestionModel &m_SpellSuggestionsModel;\
    };
    }
}

#endif // SOURCETARGETCOMMAND_H
