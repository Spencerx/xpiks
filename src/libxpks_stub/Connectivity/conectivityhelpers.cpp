/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "conectivityhelpers.h"

#include <cstddef>
#include <memory>

#include <QDebug>
#include <QString>
#include <QStringList>

#include "Artworks/artworkmetadata.h"
#include "Artworks/artworkssnapshot.h"
#include "Artworks/imageartwork.h"
#include "Common/logging.h"
#include "Encryption/secretsmanager.h"
#include "Helpers/filehelpers.h"
#include "Models/Connectivity/uploadinfo.h"
#include "Models/settingsmodel.h"

#include "Connectivity/uploadbatch.h"
#include "uploadcontext.h"

namespace Models {
    class ProxySettings;
}

#define RETRIES_COUNT 3

namespace libxpks {
    namespace net {
        void extractFilePathes(const Artworks::ArtworksSnapshot &artworksSnapshot,
                               QStringList &filePathes,
                               QStringList &zipsPathes) {

            size_t size = artworksSnapshot.size();
            filePathes.reserve(size);
            zipsPathes.reserve(size);
            LOG_DEBUG << "Generating filepathes for" << size << "item(s)";

            for (size_t i = 0; i < size; ++i) {
                auto &artwork = artworksSnapshot.get(i);
                QString filepath = artwork->getFilepath();
                filePathes.append(filepath);

                auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);

                if (image != nullptr && image->hasVectorAttached()) {
                    filePathes.append(image->getAttachedVectorPath());

                    QString zipPath = Helpers::getArchivePath(filepath);
                    zipsPathes.append(zipPath);
                } else {
                    zipsPathes.append(filepath);
                }
            }
        }

        void generateUploadContexts(const std::vector<std::shared_ptr<Models::UploadInfo> > &uploadInfos,
                                    std::vector<std::shared_ptr<UploadContext> > &contexts,
                                    Encryption::SecretsManager &secretsManager,
                                    Models::SettingsModel &settingsModel) {
            size_t size = uploadInfos.size();
            contexts.reserve(size);

            auto &proxySettings = settingsModel.getProxySettings();
            int timeoutSeconds = settingsModel.getUploadTimeout();
            const bool useProxy = settingsModel.getUseProxy();
            const bool verbose = settingsModel.getVerboseUpload();

            for (size_t i = 0; i < size; ++i) {
                std::shared_ptr<UploadContext> context(new UploadContext());
                auto &info = uploadInfos.at(i);

                context->m_Host = info->getHost();
                context->m_Username = info->getUsername();
                context->m_Password = secretsManager.decodePassword(info->getPassword());
                context->m_UsePassiveMode = !info->getDisableFtpPassiveMode();
                context->m_UseEPSV = !info->getDisableEPSV();
                context->m_UseProxy = useProxy;
                context->m_ProxySettings = &proxySettings;
                context->m_TimeoutSeconds = timeoutSeconds;
                context->m_VerboseLogging = verbose;
                context->m_VectorsFirst = info->getVectorFirst();
                // TODO: move to configs/options
                context->m_RetriesCount = RETRIES_COUNT;
                context->m_DirForImages = info->getImagesDir();
                context->m_DirForVectors = info->getVectorsDir();
                context->m_DirForVideos = info->getVideosDir();

                contexts.emplace_back(context);
            }
        }

        std::vector<std::shared_ptr<UploadBatch> > generateUploadBatches(const Artworks::ArtworksSnapshot &artworksToUpload,
                                                                         const std::vector<std::shared_ptr<Models::UploadInfo> > &uploadInfos,
                                                                         Encryption::SecretsManager &secretsManager,
                                                                         Models::SettingsModel &settingsModel) {
            LOG_DEBUG << artworksToUpload.size() << "file(s)";
            std::vector<std::shared_ptr<UploadBatch> > batches;

            QStringList filePathes;
            QStringList zipFilePathes;
            extractFilePathes(artworksToUpload, filePathes, zipFilePathes);

            std::vector<std::shared_ptr<UploadContext> > contexts;
            generateUploadContexts(uploadInfos, contexts, secretsManager, settingsModel);

            size_t size = contexts.size();
            batches.reserve(size);

            for (size_t i = 0; i < size; ++i) {
                auto &context = contexts.at(i);

                if (uploadInfos[i]->getZipBeforeUpload()) {
                    batches.emplace_back(new UploadBatch(context, zipFilePathes));
                } else {
                    batches.emplace_back(new UploadBatch(context, filePathes));
                }
            }

            return batches;
        }
    }
}
