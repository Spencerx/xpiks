#ifndef APISECRETSSTORAGE_H
#define APISECRETSSTORAGE_H

#include <QHash>
#include <QReadWriteLock>

#include "Encryption/isecretsstorage.h"
#include "Encryption/secretpair.h"

namespace libxpks {
    namespace microstocks {
        class APISecretsStorage: public Encryption::ISecretsStorage
        {
        public:
            APISecretsStorage();

            // ISecretsStorage interface
        public:
            virtual bool tryFindPair(int key, Encryption::SecretPair &secretPair) override;
            virtual bool insertPair(int key, const Encryption::SecretPair &secretPair) override;

        private:
            QReadWriteLock m_Lock;
            QHash<int, Encryption::SecretPair> m_Storage;
        };
    }
}

#endif // APISECRETSSTORAGE_H
