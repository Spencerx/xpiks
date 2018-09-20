#include "apisecretsstorage.h"
#include <Microstocks/microstockenums.h>

namespace libxpks {
    namespace microstocks {
        APISecretsStorage::APISecretsStorage()
        {
        }

        bool APISecretsStorage::tryFindPair(int key, Encryption::SecretPair &secretPair) {
            QReadLocker locker(&m_Lock);
            Q_UNUSED(locker);

            auto it = m_Storage.find(key);
            const bool found = it != m_Storage.end();
            if (found) {
                secretPair.m_Key = it->m_Key;
                secretPair.m_Value = it->m_Value;
            }

            return found;
        }

        bool APISecretsStorage::insertPair(int key, const Encryption::SecretPair &secretPair) {
            QWriteLocker locker(&m_Lock);
            Q_UNUSED(locker);

            bool success = false;

            if (!m_Storage.contains(key)) {
                m_Storage.insert(key, secretPair);
                success = true;
            }

            return success;
        }
    }
}
