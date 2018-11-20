/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "translationworker.h"

#include <string>

#include <QByteArray>
#include <QMutexLocker>
#include <QtDebug>
#include <QtGlobal>

#include <vendors/ssdll/src/ssdll/lookupdictionary.h>

#include "Common/logging.h"
#include "Common/lrucache.h"
#include "Helpers/asynccoordinator.h"
#include "Services/Translation/translationquery.h"

namespace Translation {
    TranslationWorker::TranslationWorker(Helpers::AsyncCoordinator &initCoordinator, QObject *parent) :
        QObject(parent),
        m_InitCoordinator(initCoordinator),
        m_Translations(10)
    {
    }

    TranslationWorker::~TranslationWorker() {
        // this destructor is required for unique_ptr with incomplete type
    }

    void TranslationWorker::selectDictionary(const QString &dictionaryPath) {
        LOG_INFO << dictionaryPath;

        if (m_LookupDictionary->isLoaded()) {
            LOG_INFO << "Unloading old dictionary";
            m_LookupDictionary = std::make_unique<LookupDictionary>();
        }

#ifdef Q_OS_WIN
        m_LookupDictionary->setIfoFilePath(dictionaryPath.toStdWString());
#else
        m_LookupDictionary->setIfoFilePath(dictionaryPath.toStdString());
#endif
    }

    bool TranslationWorker::retrieveTranslation(const QString &query, QString &translation) {
        QMutexLocker locker(&m_Mutex);
        Q_UNUSED(locker);
        return m_Translations.tryGet(query, translation);
    }

    bool TranslationWorker::initWorker() {
        LOG_DEBUG << "#";        

        Helpers::AsyncCoordinatorUnlocker unlocker(m_InitCoordinator);
        Q_UNUSED(unlocker);

        m_LookupDictionary = std::make_unique<LookupDictionary>();

        return true;
    }

    void TranslationWorker::processOneItem(std::shared_ptr<TranslationQuery> &item) {
        std::string translationData;
        auto &query = item->getQuery();
        LOG_INFO << "translation request:" << query;

        do {
            if (query.isEmpty()) { break; }

            ensureDictionaryLoaded();
            std::string word = query.toUtf8().toStdString();

            if (!m_LookupDictionary->translate(word, translationData)) { break; }

            QString translation = QString::fromUtf8(translationData.c_str());
            if (translation.isEmpty()) { break; }

            {
                QMutexLocker locker(&m_Mutex);
                Q_UNUSED(locker);
                m_Translations.put(query, translation);
            }
            item->notifyTranslated();
        } while (false);
    }

    void TranslationWorker::ensureDictionaryLoaded() {
        Q_ASSERT(m_LookupDictionary);

        if (!m_LookupDictionary->isLoaded()) {
            LOG_INFO << "Loading current dictionary";
            bool result = m_LookupDictionary->loadDictionary();
            if (!result) {
                LOG_WARNING << "Loading dictionary failed!";
            }
        }
    }
}
