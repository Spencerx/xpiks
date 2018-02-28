/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef WARNINGSSETTINGSMODEL_H
#define WARNINGSSETTINGSMODEL_H

#include <QJsonDocument>
#include <QJsonArray>
#include "iwarningssettings.h"
#include "../Models/abstractconfigupdatermodel.h"
#include "../Common/isystemenvironment.h"

namespace Warnings {
    class WarningsSettingsModel:
            public Models::AbstractConfigUpdaterModel,
            public IWarningsSettings
    {
        Q_OBJECT

    public:
        WarningsSettingsModel(Common::ISystemEnvironment &environment);
        void initializeConfigs();

        virtual const QString &getAllowedFilenameCharacters() const override { return m_AllowedFilenameCharacters; }
        virtual double getMinMegapixels() const override { return m_MinMegapixels; }
        virtual double getMaxImageFilesizeMB() const override { return m_MaxImageFilesizeMB; }
        virtual double getMaxVideoFilesizeMB() const override { return m_MaxVideoFilesizeMB; }
        virtual double getMinVideoDurationSeconds() const override { return m_MinVideoDurationSeconds; }
        virtual double getMaxVideoDurationSeconds() const override { return m_MaxVideoDurationSeconds; }
        virtual int getMinKeywordsCount() const override { return m_MinKeywordsCount; }
        virtual int getMaxKeywordsCount() const override { return m_MaxKeywordsCount; }
        virtual int getMinWordsCount() const override { return m_MinWordsCount; }
        virtual int getMaxDescriptionLength() const override { return m_MaxDescriptionLength; }
        virtual int getMaxTitleWords() const override { return 10; }

        // AbstractConfigUpdaterModel interface
    protected:
        virtual void processRemoteConfig(const QJsonDocument &remoteDocument, bool overwriteLocal) override;
        virtual bool processLocalConfig(const QJsonDocument &document) override;
        virtual void processMergedConfig(const QJsonDocument &document) override;

    private:
        bool parseConfig(const QJsonDocument &document);

        // CompareValuesJson interface
    public:
        virtual int operator ()(const QJsonObject &val1, const QJsonObject &val2) override;

    signals:
        void settingsUpdated();

    private:
        Common::ISystemEnvironment &m_Environment;
        QString m_AllowedFilenameCharacters;
        double m_MinMegapixels;
        double m_MaxImageFilesizeMB;
        double m_MaxVideoFilesizeMB;
        double m_MinVideoDurationSeconds;
        double m_MaxVideoDurationSeconds;
        int m_MinKeywordsCount;
        int m_MaxKeywordsCount;
        int m_MinWordsCount;
        int m_MaxDescriptionLength;
    };
}
#endif // WARNINGSSETTINGSMODEL_H
