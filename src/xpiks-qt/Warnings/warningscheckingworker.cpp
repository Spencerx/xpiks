/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "warningscheckingworker.h"
#include <QSet>
#include <QFileInfo>
#include <QSize>
#include "../Common/defines.h"
#include "../Common/flags.h"
#include "../Models/artworkmetadata.h"
#include "../Models/imageartwork.h"
#include "../Models/videoartwork.h"
#include "warningssettingsmodel.h"
#include "warningsitem.h"

#define WARNINGS_WORKER_SLEEP_INTERVAL 500
#define WARNINGS_DELAY_PERIOD 50

namespace Warnings {
    QSet<QString> toLowerSet(const QStringList &from) {
        QSet<QString> result;
        result.reserve(from.length());
        foreach(const QString &str, from) {
            result.insert(str.toLower());
        }
        return result;
    }

    WarningsCheckingWorker::WarningsCheckingWorker(WarningsSettingsModel *warningsSettingsModel,
                                                   QObject *parent):
        QObject(parent),
        ItemProcessingWorker(WARNINGS_DELAY_PERIOD),
        m_WarningsSettingsModel(warningsSettingsModel)
    {
        Q_ASSERT(warningsSettingsModel != nullptr);
    }

    bool WarningsCheckingWorker::initWorker() {
        LOG_DEBUG << "#";
        return true;
    }

    void WarningsCheckingWorker::processOneItemEx(std::shared_ptr<IWarningsItem> &item, batch_id_t batchID, Common::flag_t flags) {
        if (getIsSeparatorFlag(flags)) {
            emit queueIsEmpty();
        } else {
            ItemProcessingWorker::processOneItemEx(item, batchID, flags);

            if (getWithDelayFlag(flags)) {
                QThread::msleep(WARNINGS_WORKER_SLEEP_INTERVAL);
            }
        }
    }

    void WarningsCheckingWorker::processOneItem(std::shared_ptr<IWarningsItem> &item) {
        std::shared_ptr<WarningsItem> warningItem = std::dynamic_pointer_cast<WarningsItem>(item);
        Q_ASSERT(warningItem);

        if (warningItem) {
            processWarningsItem(warningItem);
        }
    }

    void WarningsCheckingWorker::processWarningsItem(std::shared_ptr<WarningsItem> &item) {
        Common::flag_t warningsFlags = 0;

        if (item->needCheckAll()) {
            warningsFlags |= checkDimensions(item);
            warningsFlags |= checkDescription(item);
            warningsFlags |= checkTitle(item);
            warningsFlags |= checkKeywords(item);
        } else {
            auto checkingFlags = item->getCheckingFlags();
            switch (checkingFlags) {
                case Common::WarningsCheckFlags::Description:
                    warningsFlags |= checkDescription(item);
                    break;
                case Common::WarningsCheckFlags::Keywords:
                    warningsFlags |= checkKeywords(item);
                    warningsFlags |= checkDuplicates(item);
                    break;
                case Common::WarningsCheckFlags::Title:
                    warningsFlags |= checkTitle(item);
                    break;
                case Common::WarningsCheckFlags::Spelling:
                    warningsFlags |= checkSpelling(item);
                    break;
                case Common::WarningsCheckFlags::All:
                    // to make compiler happy
                    break;
            }
        }

        item->submitWarnings(warningsFlags);
    }

    Common::flag_t WarningsCheckingWorker::checkDimensions(std::shared_ptr<WarningsItem> &wi) const {
        LOG_INTEGRATION_TESTS << "#";
        const QString &allowedFilenameCharacters = m_WarningsSettingsModel->getAllowedFilenameCharacters();
        double minimumMegapixels = m_WarningsSettingsModel->getMinMegapixels();

        Models::ArtworkMetadata *item = wi->getCheckableItem();
        Common::flag_t warningsInfo = 0;

        Models::ImageArtwork *image = dynamic_cast<Models::ImageArtwork *>(item);
        if (image != NULL) {
            QSize size = image->getImageSize();

            double currentProd = size.width() * size.height() / 1000000.0;
            if ((0.1 < currentProd) && (currentProd < minimumMegapixels)) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::SizeLessThanMinimum);
            }
        }

        Models::VideoArtwork *video = dynamic_cast<Models::VideoArtwork *>(item);

        qint64 filesize = item->getFileSize();
        double filesizeMB = (double)filesize;
        filesizeMB /= (1024.0*1024.0);
        if (image != NULL) {
            double maxImageFileSizeMB = m_WarningsSettingsModel->getMaxImageFilesizeMB();
            if (filesizeMB >= maxImageFileSizeMB) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::ImageFileIsTooBig);
            }
        } else if (video != NULL) {
            double maxVideoFileSizeMB = m_WarningsSettingsModel->getMaxVideoFilesizeMB();
            if (filesizeMB >= maxVideoFileSizeMB) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::VideoFileIsTooBig);
            }
        }

        if (video != nullptr) {
            const double duration = video->getDuration();
            const double maxDuration = m_WarningsSettingsModel->getMaxVideoDurationSeconds();
            const double minDuration = m_WarningsSettingsModel->getMinVideoDurationSeconds();

            if (duration > maxDuration) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::VideoIsTooLong);
            }

            if (duration < minDuration) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::VideoIsTooShort);
            }
        }

        QFileInfo fi(item->getFilepath());
        QString filename = fi.fileName();
        int length = filename.length();
        for (int i = 0; i < length; ++i) {
            QChar c = filename[i];
            bool isOk = c.isLetter() || c.isDigit() ||
                        allowedFilenameCharacters.contains(c);
            if (!isOk) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::FilenameSymbols);
                break;
            }
        }

        return warningsInfo;
    }

    Common::flag_t WarningsCheckingWorker::checkKeywords(std::shared_ptr<WarningsItem> &wi) const {
        LOG_INTEGRATION_TESTS << "#";
        int minimumKeywordsCount = m_WarningsSettingsModel->getMinKeywordsCount();
        int maximumKeywordsCount = m_WarningsSettingsModel->getMaxKeywordsCount();
        Common::flag_t warningsInfo = 0;
        Models::ArtworkMetadata *item = wi->getCheckableItem();
        Common::BasicKeywordsModel *keywordsModel = item->getBasicModel();

        int keywordsCount = keywordsModel->getKeywordsCount();

        if (keywordsCount == 0) {
            Common::SetFlag(warningsInfo, Common::WarningFlags::NoKeywords);
        } else {
            if (keywordsCount < minimumKeywordsCount) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::TooFewKeywords);
            }

            if (keywordsCount > maximumKeywordsCount) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::TooManyKeywords);
            }

            if (keywordsModel->hasKeywordsSpellError()) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::SpellErrorsInKeywords);
            }
        }

        return warningsInfo;
    }

    Common::flag_t WarningsCheckingWorker::checkDescription(std::shared_ptr<WarningsItem> &wi) const {
        LOG_INTEGRATION_TESTS << "#";
        int maximumDescriptionLength = m_WarningsSettingsModel->getMaxDescriptionLength();
        Common::flag_t warningsInfo = 0;
        Models::ArtworkMetadata *item = wi->getCheckableItem();

        int descriptionLength = wi->getDescription().length();

        if (descriptionLength == 0) {
            Common::SetFlag(warningsInfo, Common::WarningFlags::DescriptionIsEmpty);
        } else {
            auto *keywordsModel = item->getBasicModel();

            if (descriptionLength > maximumDescriptionLength) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::DescriptionTooBig);
            }

            QStringList descriptionWords = wi->getDescriptionWords();

            int wordsLength = descriptionWords.length();
            int minWordsCount = m_WarningsSettingsModel->getMinWordsCount();
            if (wordsLength < minWordsCount) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::DescriptionNotEnoughWords);
            }

            if (keywordsModel->hasDescriptionSpellError()) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::SpellErrorsInDescription);
            }

            QSet<QString> descriptionWordsSet = toLowerSet(descriptionWords);
            QSet<QString> intersection = descriptionWordsSet.intersect(item->getKeywordsSet());

            if (!intersection.isEmpty()) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::KeywordsInDescription);
            }
        }

        return warningsInfo;
    }

    Common::flag_t WarningsCheckingWorker::checkTitle(std::shared_ptr<WarningsItem> &wi) const {
        LOG_INTEGRATION_TESTS << "#";

        Common::flag_t warningsInfo = 0;
        Models::ArtworkMetadata *item = wi->getCheckableItem();

        int titleLength = wi->getTitle().length();

        if (titleLength == 0) {
            Common::SetFlag(warningsInfo, Common::WarningFlags::TitleIsEmpty);
        } else {
            auto *keywordsModel = item->getBasicModel();

            QStringList titleWords = wi->getTitleWords();
            int partsLength = titleWords.length();

            int minWordsCount = m_WarningsSettingsModel->getMinWordsCount();
            if (partsLength < minWordsCount) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::TitleNotEnoughWords);
            }

            if (partsLength > 10) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::TitleTooManyWords);
            }

            if (keywordsModel->hasTitleSpellError()) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::SpellErrorsInTitle);
            }

            QSet<QString> titleWordsSet = toLowerSet(titleWords);
            QSet<QString> intersection = titleWordsSet.intersect(item->getKeywordsSet());

            if (!intersection.isEmpty()) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::KeywordsInTitle);
            }
        }

        return warningsInfo;
    }

    Common::flag_t WarningsCheckingWorker::checkSpelling(std::shared_ptr<WarningsItem> &wi) const {
        LOG_INTEGRATION_TESTS << "#";

        Common::flag_t warningsInfo = 0;
        Models::ArtworkMetadata *item = wi->getCheckableItem();
        auto *keywordsModel = item->getBasicModel();

        if (keywordsModel->hasKeywordsSpellError()) {
            LOG_INTEGRATION_TESTS << "Detected keywords spell error";
            Common::SetFlag(warningsInfo, Common::WarningFlags::SpellErrorsInKeywords);
        }

        if (keywordsModel->hasDescriptionSpellError()) {
            LOG_INTEGRATION_TESTS << "Detected description spell error";
            Common::SetFlag(warningsInfo, Common::WarningFlags::SpellErrorsInDescription);
        }

        if (keywordsModel->hasTitleSpellError()) {
            LOG_INTEGRATION_TESTS << "Detected title spell error";
            Common::SetFlag(warningsInfo, Common::WarningFlags::SpellErrorsInTitle);
        }

        return warningsInfo;
    }

    Common::flag_t WarningsCheckingWorker::checkDuplicates(std::shared_ptr<WarningsItem> &wi) const {
        Common::flag_t warningsInfo = 0;
        Models::ArtworkMetadata *item = wi->getCheckableItem();
        Common::BasicKeywordsModel *keywordsModel = item->getBasicModel();

        if (keywordsModel->getKeywordsCount() == 0) {
            return warningsInfo;
        }

        const QSet<QString> &keywordsSet = wi->getKeywordsSet();

        QStringList titleWords = wi->getTitleWords();
        if (!titleWords.isEmpty()) {
            QSet<QString> titleWordsSet = toLowerSet(titleWords);
            QSet<QString> intersection = titleWordsSet.intersect(keywordsSet);

            if (!intersection.isEmpty()) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::KeywordsInTitle);
            }
        }

        QStringList descriptionWords = wi->getDescriptionWords();
        if (!descriptionWords.isEmpty()) {
            QSet<QString> descriptionWordsSet = toLowerSet(descriptionWords);
            QSet<QString> intersection = descriptionWordsSet.intersect(keywordsSet);

            if (!intersection.isEmpty()) {
                Common::SetFlag(warningsInfo, Common::WarningFlags::KeywordsInDescription);
            }
        }

        return warningsInfo;
    }
}
