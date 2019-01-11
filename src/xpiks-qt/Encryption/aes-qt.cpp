/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "aes-qt.h"

#include <cstdint>

#include <QByteArray>
#include <QChar>
#include <QCryptographicHash>
#include <QString>
#include <QtGlobal>

#include <vendors/tiny-aes/aes.h>

#define MAX_ENCRYPTION_LENGTH 2048

namespace Encryption {

    const uint8_t iv[]  = { 0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87, 0x78, 0x69, 0x5a, 0x4b, 0x3c, 0x2d, 0x5e, 0xaf };

    inline int getAlignedSize(int currSize, int alignment) {
        Q_ASSERT(currSize >= 0);

        int padding = (alignment - currSize % alignment) % alignment;
        return currSize + padding;
    }

    QString encodeText(const QString &rawText, const QString &key) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(key.toUtf8());
        QByteArray keyData = hash.result();

        const ushort *rawData = rawText.utf16();
        void *rawDataVoid = (void*)rawData;
        const char *rawDataChar = static_cast<const char*>(rawDataVoid);
        QByteArray inputData;
        inputData.append(rawDataChar, rawText.size() * sizeof(QChar) + 1);

        const int length = inputData.size();
        int encryptionLength = getAlignedSize(length, 16);
        Q_ASSERT(encryptionLength % 16 == 0 && encryptionLength >= length);

        QByteArray encodingBuffer(encryptionLength, '\0');
        inputData.resize(encryptionLength);
        for (int i = length; i < encryptionLength; i++) { inputData[i] = 0; }

        AES_CBC_encrypt_buffer((uint8_t*)encodingBuffer.data(), (uint8_t*)inputData.data(), encryptionLength, (const uint8_t*)keyData.data(), iv);

        QByteArray data(encodingBuffer.data(), encryptionLength);
        QString hex = QString::fromLatin1(data.toHex());
        return hex;
    }

    QString decodeText(const QString &hexEncodedText, const QString &key) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        hash.addData(key.toUtf8());
        QByteArray keyData = hash.result();

        const int length = hexEncodedText.size();
        int encryptionLength = getAlignedSize(length, 16);

        QByteArray encodingBuffer(encryptionLength, '\0');

        QByteArray encodedText = QByteArray::fromHex(hexEncodedText.toLatin1());
        const int encodedOriginalSize = encodedText.size();
        Q_ASSERT(encodedText.length() <= encryptionLength);
        encodedText.resize(encryptionLength);
        for (int i = encodedOriginalSize; i < encryptionLength; i++) { encodedText[i] = 0; }

        AES_CBC_decrypt_buffer((uint8_t*)encodingBuffer.data(), (uint8_t*)encodedText.data(), encryptionLength, (const uint8_t*)keyData.data(), iv);

        encodingBuffer.append("\0\0");
        void *data = encodingBuffer.data();
        const ushort *decodedData = static_cast<ushort *>(data);
        QString result = QString::fromUtf16(decodedData, -1);
        return result;
    }
}
