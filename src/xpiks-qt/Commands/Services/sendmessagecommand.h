/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SENDMESSAGECOMMAND_H
#define SENDMESSAGECOMMAND_H

#include "Commands/Base/icommand.h"
#include "Commands/Base/icommandtemplate.h"
#include "Common/messages.h"
#include "Common/types.h"
#include "Artworks/artworkssnapshot.h"

namespace Commands {
    template<class MessageType>
    class SendMessageCommand: public ICommand, public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        SendMessageCommand(Common::MessagesSource<Common::NamedType<MessageType>> &source, MessageType value):
            m_Source(source),
            m_Value(value)
        { }

        // ICommand interface
    public:
        virtual void execute() override {
            m_Source.sendMessage(m_Value);
        }
        virtual void undo() override { /*DO NOTHING*/ }

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(const Artworks::ArtworksSnapshot &) override { execute(); }
        virtual void undo(const Artworks::ArtworksSnapshot &) override {}

    private:
        Common::MessagesSource<Common::NamedType<MessageType>> &m_Source;
        MessageType m_Value;
    };
}

#endif // SENDMESSAGECOMMAND_H
