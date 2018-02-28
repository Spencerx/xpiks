/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef WARNINGSQUERYITEM
#define WARNINGSQUERYITEM

#include "../Common/flags.h"
#include <QSet>
#include "iwarningsitem.h"

namespace Models {
    class ArtworkMetadata;
}

namespace Warnings {
    class IWarningsSettings;

    class WarningsItem: public IWarningsItem {
    public:
        WarningsItem(Models::ArtworkMetadata *checkableItem, Common::WarningsCheckFlags checkingFlags = Common::WarningsCheckFlags::All);
        virtual ~WarningsItem();

    public:
        virtual void checkWarnings(IWarningsSettings *warningsSettings) override;

    private:
        void submitWarnings();

    private:
        void checkImageProperties(IWarningsSettings *warningsSettings);
        void checkVideoProperties(IWarningsSettings *warningsSettings);
        void checkFilename(IWarningsSettings *warningsSettings);
        void checkKeywords(IWarningsSettings *warningsSettings);
        void checkDescription(IWarningsSettings *warningsSettings);
        void checkTitle(IWarningsSettings *warningsSettings);
        void checkSpelling(IWarningsSettings *warningsSettings);
        void checkDuplicates(IWarningsSettings *warningsSettings);

    private:
        bool needCheckAll() const { return m_CheckingFlags == Common::WarningsCheckFlags::All; }
        Common::WarningsCheckFlags getCheckingFlags() const { return m_CheckingFlags; }
        QString getDescription() const;
        QString getTitle() const;
        QSet<QString> getKeywordsSet() const;

    private:
        void accountFlag(Common::WarningFlags flag, bool value);
        void dropFlag(Common::WarningFlags flag);

    private:
        QStringList getDescriptionWords() const;
        QStringList getTitleWords() const;

    private:
        Models::ArtworkMetadata *m_CheckableItem;
        Common::WarningsCheckFlags m_CheckingFlags;
        Common::WarningFlags m_FlagsToSet;
        Common::WarningFlags m_FlagsToDrop;
    };
}

#endif // WARNINGSQUERYITEM

