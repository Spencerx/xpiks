/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef PRESETMODEL_H
#define PRESETMODEL_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "Artworks/basickeywordsmodel.h"
#include "Artworks/ibasicmodelsource.h"
#include "Common/flags.h"
#include "Common/types.h"
#include "KeywordsPresets/groupmodel.h"

namespace KeywordsPresets {
    typedef unsigned int ID_t;

    struct PresetModel: public Artworks::IBasicModelSource {
        enum PresetModelFlags {
            FlagIsNameDupcate = 1 << 0
        };

        PresetModel(ID_t id):
            m_PresetName(QObject::tr("Untitled")),
            m_ID(id),
            m_GroupID(DEFAULT_GROUP_ID),
            m_Flags(0)
        {
        }

        PresetModel(ID_t id, const QString &name):
            m_PresetName(name),
            m_ID(id),
            m_GroupID(DEFAULT_GROUP_ID),
            m_Flags(0)
        {}

        PresetModel(ID_t id, const QString &name, const QStringList &keywords, int groupID):
            m_PresetName(name),
            m_ID(id),
            m_GroupID(groupID),
            m_Flags(0)
        {
            m_KeywordsModel.setKeywords(keywords);
        }

        // IBasicModelSource interface
    public:
        virtual Artworks::BasicKeywordsModel &getBasicModel() override { return m_KeywordsModel; }

        inline bool getIsNameDuplicateFlag() const { return Common::HasFlag(m_Flags, FlagIsNameDupcate); }
        inline void setIsNameDuplicateFlag(bool value) { Common::ApplyFlag(m_Flags, value, FlagIsNameDupcate); }

        Artworks::BasicKeywordsModel m_KeywordsModel;
        QString m_PresetName;
        ID_t m_ID;
        int m_GroupID;
        Common::flag_t m_Flags;
    };
}

#endif // PRESETMODEL_H
