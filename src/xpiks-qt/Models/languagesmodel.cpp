/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "languagesmodel.h"

#include <QByteArray>
#include <QChar>
#include <QCharRef>
#include <QCoreApplication>
#include <QFileInfo>
#include <QFileInfoList>
#include <QLatin1String>
#include <QLocale>
#include <QString>
#include <QStringList>
#include <QTranslator>
#include <QtDebug>

#include "Common/logging.h"
#include "Models/settingsmodel.h"

namespace Models {
    LanguagesModel::LanguagesModel(SettingsModel &settingsModel, QObject *parent):
        QAbstractListModel(parent),
        m_SettingsModel(settingsModel),
        m_CurrentLanguageIndex(-1)
    {
        m_XpiksTranslator = new QTranslator(this);
        m_QtTranslator = new QTranslator(this);

        QString translationsPath = QCoreApplication::applicationDirPath();;

#if defined(Q_OS_MAC)
        translationsPath += "/../Resources/translations/";
#else
        translationsPath += "/translations/";
#endif
        m_TranslationsPath = QDir::cleanPath(translationsPath);
    }

    void LanguagesModel::initFirstLanguage() {
        QString selectedLocale = m_SettingsModel.getSelectedLocale();

        QDir languagesDir(m_TranslationsPath);
        QCoreApplication *app = QCoreApplication::instance();
        Q_UNUSED(app);

        QString qtTranslatorPath = languagesDir.filePath(QString("qt_%1.qm").arg(selectedLocale));
        if (m_QtTranslator->load(qtTranslatorPath)) {
            LOG_DEBUG << "Loaded" << qtTranslatorPath;
            app->installTranslator(m_QtTranslator);
        }

        QString xpiksTranslatorPath = languagesDir.filePath(QString("xpiks_%1.qm").arg(selectedLocale));
        if (m_XpiksTranslator->load(xpiksTranslatorPath)) {
            LOG_DEBUG << "Loaded" << xpiksTranslatorPath;
            app->installTranslator(m_XpiksTranslator);
        } else {
            selectedLocale = "en_US";

            if (selectedLocale != m_SettingsModel.getSelectedLocale()) {
                m_SettingsModel.setSelectedLocale(selectedLocale);
                m_SettingsModel.saveLocale();
            }

            xpiksTranslatorPath = languagesDir.filePath(QLatin1String("xpiks_en_US.qm"));
            if (m_XpiksTranslator->load(xpiksTranslatorPath)) {
                LOG_DEBUG << "Loaded" << xpiksTranslatorPath;
                app->installTranslator(m_XpiksTranslator);
            }
        }

        LOG_INFO << "Initialized with" << selectedLocale;
        emit languageChanged();
    }

    QString LanguagesModel::getCurrentLanguage() const {
        if (0 <= m_CurrentLanguageIndex && m_CurrentLanguageIndex < m_LanguagesList.length()) {
            return m_LanguagesList[m_CurrentLanguageIndex].first;
        } else {
            return QString();
        }
    }

    void LanguagesModel::loadLanguages() {
        QString selectedLocale = m_SettingsModel.getSelectedLocale();

        LOG_DEBUG << "Current locale is" << selectedLocale;
        loadTranslators(QDir(m_TranslationsPath), selectedLocale);
    }

    void LanguagesModel::switchLanguage(int index) {
        LOG_DEBUG << "index" << index;
        if (index == m_CurrentLanguageIndex) { return; }

        const QPair<QString, QString> &langPair = m_LanguagesList.at(index);
        m_CurrentLanguageIndex = index;

        QDir languagesDir(m_TranslationsPath);

        m_SettingsModel.setSelectedLocale(langPair.first);
        m_SettingsModel.saveLocale();

        QCoreApplication *app = QCoreApplication::instance();
        Q_UNUSED(app);
        app->removeTranslator(m_QtTranslator);
        app->removeTranslator(m_XpiksTranslator);

        QString qtTranslatorPath = languagesDir.filePath(QString("qt_%1.qm").arg(langPair.first));
        if (m_QtTranslator->load(qtTranslatorPath)) {
            LOG_DEBUG << "Loaded" << qtTranslatorPath;
            app->installTranslator(m_QtTranslator);
        }

        QString xpiksTranslatorPath = languagesDir.filePath(QString("xpiks_%1.qm").arg(langPair.first));
        if (m_XpiksTranslator->load(xpiksTranslatorPath)) {
            LOG_DEBUG << "Loaded" << xpiksTranslatorPath;
            app->installTranslator(m_XpiksTranslator);
        }

        LOG_INFO << "Switched to" << langPair.first;
        emit languageChanged();
        emit dataChanged(this->index(0), this->index(m_LanguagesList.length() - 1));
    }
    
    QVariant LanguagesModel::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if (row < 0 || row >= m_LanguagesList.length()) return QVariant();
        if (role == Qt::DisplayRole) { return m_LanguagesList.at(index.row()).second; }
        else if (role == IsCurrentRole) { return row == m_CurrentLanguageIndex; }
        return QVariant();
    }

    QHash<int, QByteArray> LanguagesModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[IsCurrentRole] = "iscurrent";
        roles[Qt::DisplayRole] = "display";
        return roles;
    }

    void LanguagesModel::loadTranslators(const QDir &dir, const QString &selectedLocale) {
        LOG_DEBUG << dir.absolutePath();
        
        const QString filter = QLatin1String("xpiks_*.qm");
        QDir::Filters filters = QDir::Files | QDir::Readable;
        QDir::SortFlags sort = QDir::Name;
        QFileInfoList entries = dir.entryInfoList(QStringList() << filter, filters, sort);

        QVector<QPair<QString, QString> > foundLocales;

        int size = entries.size();
        for (int i = 0; i < size; ++i) {
            const QFileInfo &file = entries.at(i);
            QStringList parts = file.baseName().split(QChar('_'));
            QString language = parts.at(parts.count() - 2);
            QString country  = parts.at(parts.count() - 1);

            //QTranslator* translator = new QTranslator(this);
            //if (translator->load(file.absoluteFilePath())) {
            QString locale = language + "_" + country;

            if (selectedLocale == locale) { m_CurrentLanguageIndex = i; }

            QLocale localeVar(locale);
            QString lang = QLocale::languageToString(localeVar.language());
            QString nativeLang = localeVar.nativeLanguageName();
            nativeLang.replace(0, 1, nativeLang[0].toUpper());

            foundLocales << qMakePair(locale, QString("%1 (%2)").arg(nativeLang).arg(lang));
                
            LOG_INFO << "Found" << locale << "translation for language:" << lang;
        }

        beginResetModel();
        m_LanguagesList = foundLocales;
        endResetModel();
    }
}

