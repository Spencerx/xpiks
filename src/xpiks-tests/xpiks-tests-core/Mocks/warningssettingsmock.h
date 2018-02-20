#ifndef WARNINGSSETTINGSMOCK_H
#define WARNINGSSETTINGSMOCK_H

#include "../../xpiks-qt/Warnings/iwarningssettings.h"

namespace Mocks {
    class WarningsSettingsMock: public Warnings::IWarningsSettings {
    public:
        WarningsSettingsMock():
            m_AllowedFilenameCharacters("._-@#"),
            m_MinMegapixels(4.0),
            m_MaxImageFilesizeMB(50),
            m_MaxVideoFilesizeMB(4096),
            m_MinVideoDurationSeconds(5),
            m_MaxVideoDurationSeconds(60),
            m_MinKeywordsCount(7),
            m_MaxKeywordsCount(50),
            m_MinWordsCount(3),
            m_MaxTitleWordsCount(10),
            m_MaxDescriptionLength(200)
        {}

    public:
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
        virtual int getMaxTitleWords() const override { return m_MaxTitleWordsCount; }

    private:
        QString m_AllowedFilenameCharacters;
        double m_MinMegapixels;
        double m_MaxImageFilesizeMB;
        double m_MaxVideoFilesizeMB;
        double m_MinVideoDurationSeconds;
        double m_MaxVideoDurationSeconds;
        int m_MinKeywordsCount;
        int m_MaxKeywordsCount;
        int m_MinWordsCount;
        int m_MaxTitleWordsCount;
        int m_MaxDescriptionLength;
    };
}

#endif // WARNINGSSETTINGSMOCK_H
