/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "simplecurlrequest.h"

#include <string>
#include <cstring>
#include <cstdlib>

#include <QtDebug>
#include <QtGlobal>

#include <curl/curl.h>

#include "Common/logging.h"
#include "Connectivity/curlhelpers.h"

struct MemoryStruct {
    char *m_Memory;
    size_t m_Size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    mem->m_Memory = (char *)realloc(mem->m_Memory, mem->m_Size + realsize + 1);
    if(mem->m_Memory == NULL) {
        /* out of memory! */
        return 0;
    }

    memcpy(&(mem->m_Memory[mem->m_Size]), contents, realsize);
    mem->m_Size += realsize;
    mem->m_Memory[mem->m_Size] = 0;

    return realsize;
}

namespace Connectivity {
    SimpleCurlRequest::SimpleCurlRequest(const QString &resource, bool verifySSL, QObject *parent) :
        QObject(parent),
        m_RemoteResource(resource),
        m_ProxySettings(nullptr),
        m_VerifySSL(verifySSL)
    {
    }

    bool SimpleCurlRequest::sendRequestSync() {
        return doRequest();
    }

    void SimpleCurlRequest::addRawHeaders(const QStringList &headers) {
        m_RawHeaders.append(headers);
    }

    void SimpleCurlRequest::setProxySettings(const Models::ProxySettings *proxySettings) {
        m_ProxySettings = proxySettings;
    }

    void SimpleCurlRequest::process() {
        bool success = doRequest();
        emit requestFinished(success);
    }

    bool SimpleCurlRequest::doRequest() {
        // https://curl.haxx.se/libcurl/c/getinmemory.html
        CURL *curl_handle = nullptr;
        CURLcode res;

        MemoryStruct chunk;
        chunk.m_Memory = nullptr;
        chunk.m_Size = 0;

        struct curl_slist *curl_headers = NULL;

        /* init the curl session */
        curl_handle = curl_easy_init();

        if (!curl_handle) {
            return false;
        }

        std::string resourceString = m_RemoteResource.toStdString();
        const char *url = resourceString.data();
        /* specify URL to get */
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);

        if (!m_VerifySSL) {
            /*
             * If you want to connect to a site who isn't using a certificate that is
             * signed by one of the certs in the CA bundle you have, you can skip the
             * verification of the server's certificate. This makes the connection
             * A LOT LESS SECURE.
             *
             * If you have a CA cert for the server stored someplace else than in the
             * default bundle, then the CURLOPT_CAPATH option might come handy for
             * you.
            */
            curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);

            /*
             * If the site you're connecting to uses a different host name that what
             * they have mentioned in their server certificate's commonName (or
             * subjectAltName) fields, libcurl will refuse to connect. You can skip
             * this check, but this will make the connection less secure.
            */
            curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
        }

        /* send all data to this function  */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

        /* some servers don't like requests that are made without a user-agent
             field, so we provide one */
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        if (!m_RawHeaders.empty()) {
            foreach (const QString &header, m_RawHeaders) {
                std::string headerString = header.toStdString();
                curl_headers = curl_slist_append(curl_headers, headerString.data());
            }

            /* set our custom set of headers */
            curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, curl_headers);
        }

        if (m_ProxySettings != nullptr) {
            fillProxySettings(curl_handle, m_ProxySettings);
        }

        /* get it! */
        res = curl_easy_perform(curl_handle);

        const bool success = (CURLE_OK == res);

        /* check for errors */
        if (!success) {
            m_ErrorString = QString::fromLatin1(curl_easy_strerror(res));
            LOG_WARNING << "curl_easy_perform() failed" << m_ErrorString;
        } else {
            /*
             * Now, our chunk.memory points to a memory block that is chunk.size
             * bytes big and contains the remote file.
             *
             * Do something nice with it!
             */

            LOG_INFO << chunk.m_Size << "bytes received";
            m_ResponseData = QByteArray(chunk.m_Memory, (uint)chunk.m_Size);
        }

        /* cleanup curl stuff */
        curl_easy_cleanup(curl_handle);

        /* free the custom headers */
        if (curl_headers != NULL) {
            curl_slist_free_all(curl_headers);
        }

        free(chunk.m_Memory);

        return success;
    }
}
