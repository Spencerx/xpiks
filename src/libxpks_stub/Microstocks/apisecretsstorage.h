#ifndef APISECRETSSTORAGE_H
#define APISECRETSSTORAGE_H

#include <Encryption/isecretsstorage.h>
#include <QHash>
#include <QReadWriteLock>

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
