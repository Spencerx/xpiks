/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "csvexportworker.h"

#include <cstddef>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QRegularExpression>
#include <QThread>
#include <QtDebug>
#include <QtGlobal>

#include "Artworks/artworkmetadata.h"
#include "Artworks/artworkssnapshot.h"
#include "Common/logging.h"
#include "Helpers/stringhelper.h"
#include "MetadataIO/csvexportproperties.h"

#define DOUBLE_QUOTE "\""
#define CSV_SEPARATOR ","

void writeCsvValue(QFile &csvFile, const QString &value) {
    Q_ASSERT(value == value.trimmed());
    const bool needsQuotes = Helpers::needsCsvQuotes(value);

    if (needsQuotes) { csvFile.write(DOUBLE_QUOTE); }
    {
        csvFile.write(value.toUtf8());
    }
    if (needsQuotes) { csvFile.write(DOUBLE_QUOTE); }
}

namespace MetadataIO {
    void writeColumnNames(QFile &csvFile, const std::shared_ptr<CsvExportPlan> &plan) {
        auto &properties = plan->m_PropertiesToExport;

        const size_t size = properties.size();
        Q_ASSERT(size != 0);

        QString columnName = properties[0].m_ColumnName;
        writeCsvValue(csvFile, columnName);

        for (size_t i = 1; i < size; ++i) {
            columnName = properties[i].m_ColumnName;

            csvFile.write(CSV_SEPARATOR);
            writeCsvValue(csvFile, columnName);
        }

        // rfc 4180 - eol should be dos-style
        csvFile.write("\r\n");
    }

    QString retrieveArtworkProperty(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork,
                                    CsvExportPropertyType property) {
        switch (property) {
        case Empty: return QString();
        case Filename: return artwork->getBaseFilename();
        case Title: return artwork->getTitle();
        case Description: return artwork->getDescription();
        case Keywords: return artwork->getKeywordsString();
        case Category1: return QString();
        case Category2: return QString();
        default: return QString();
        }
    }

    void writeRows(QFile &csvFile, const std::shared_ptr<CsvExportPlan> &plan,
                   const Artworks::ArtworksSnapshot::Container &artworks) {
        auto &properties = plan->m_PropertiesToExport;
        const size_t propertiesSize = properties.size();
        Q_ASSERT(propertiesSize != 0);

        for (auto &artwork: artworks) {
            QString value = retrieveArtworkProperty(artwork, properties[0].m_PropertyType);
            writeCsvValue(csvFile, value);

            for (size_t i = 1; i < propertiesSize; ++i) {
                value = retrieveArtworkProperty(artwork, properties[i].m_PropertyType);
                csvFile.write(CSV_SEPARATOR);
                writeCsvValue(csvFile, value);
            }

            // rfc 4180 - eol should be dos-style
            csvFile.write("\r\n");
        }
    }

    void runExportPlan(const std::shared_ptr<CsvExportPlan> &plan, const QString &filepath,
                       const Artworks::ArtworksSnapshot::Container &artworks) {
        LOG_DEBUG << "Plan" << plan->m_Name << ": exporting" << artworks.size() << "artwork(s) to" << filepath;
        Q_ASSERT(plan->m_IsSelected);

        QFile csvFile(filepath);
        if (csvFile.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
            writeColumnNames(csvFile, plan);
            writeRows(csvFile, plan, artworks);

            csvFile.flush();
            csvFile.close();
        } else {
            LOG_WARNING << "Failed to open" << filepath;
        }
    }

    QString filenameForPlan(const std::shared_ptr<CsvExportPlan> &plan) {
        Q_ASSERT(!plan->m_Name.trimmed().isEmpty());
#ifndef INTEGRATION_TESTS
        QString time = QDateTime::currentDateTimeUtc().toString("ddMMyyyy-hhmmss");
#else
        QString time = "now";
#endif

        QString sanitizedName = plan->m_Name;
        sanitizedName.replace(QRegularExpression("[^a-zA-Z0-9 -]+"), "_");
        if (!sanitizedName.contains(QRegularExpression("[a-zA-Z0-9 -]+")) ||
                sanitizedName.isEmpty()) {
            sanitizedName = "plan" + sanitizedName;
        }

        QString result = QString("%1-%2-xpks.csv").arg(sanitizedName).arg(time);
        return result;
    }

    CsvExportWorker::CsvExportWorker(const std::vector<std::shared_ptr<CsvExportPlan> > &exportPlans,
                                     Artworks::ArtworksSnapshot &selectedArtworks,
                                     const QString &exportDirectoryPath,
                                     QObject *parent):
        QObject(parent),
        m_ExportPlans(exportPlans),
        m_ArtworksToExport(selectedArtworks),
        m_ExportDirectoryPath(exportDirectoryPath)
    {
    }

    void CsvExportWorker::doWork() {
        LOG_DEBUG << "#";

        QDir directory(m_ExportDirectoryPath);
        Q_ASSERT(directory.exists());
        if (!directory.exists()) {
            LOG_WARNING << "Directory" << m_ExportDirectoryPath << "does not exist. Aborting export...";
            return;
        }

        for (auto &plan: m_ExportPlans) {
            if (plan->m_IsSelected) {
                Q_ASSERT(!plan->m_PropertiesToExport.empty());
                if (plan->m_PropertiesToExport.empty()) {
                    LOG_WARNING << "Plan" << plan->m_Name << "has no properties to export. Skipping...";
                    continue;
                }

                const QString filename = filenameForPlan(plan);
                QString fullFilePath = directory.filePath(filename);

                try {
                    runExportPlan(plan, fullFilePath, m_ArtworksToExport.getRawData());
                } catch(...) {
                    LOG_WARNING << "Exception while exporting" << plan->m_Name << "to CSV";
                }
            }
        }

        if ((m_ExportPlans.size() * m_ArtworksToExport.size()) < 1000) {
            // simulate working for the spinner to show up
            QThread::msleep(1000);
        }
    }
}
