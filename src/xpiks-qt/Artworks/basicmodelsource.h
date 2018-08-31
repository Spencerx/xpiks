/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef BASICMODELSOURCE_H
#define BASICMODELSOURCE_H

#include "ibasicmodelsource.h"

namespace Artworks {
    class BasicKeywordsModel;

    class BasicModelSource: public IBasicModelSource
    {
    public:
        BasicModelSource(BasicKeywordsModel &basicModel):
            m_BasicModel(basicModel)
        {}

        // IBasicModelSource interface
    public:
        virtual BasicKeywordsModel &getBasicModel() override { return m_BasicModel; }

    private:
        BasicKeywordsModel &m_BasicModel;
    };
}

#endif // BASICMODELSOURCE_H
