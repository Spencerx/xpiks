/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IREFCOUNTEDOBJECT_H
#define IREFCOUNTEDOBJECT_H

namespace Common {
    class IRefCountedObject {
    public:
        virtual ~IRefCountedObject() {}
        virtual void acquire() = 0;
        virtual bool release() = 0;
    };

    template<class T>
    class HoldLocker {
    public:
    public:
        HoldLocker(T *obj):
            m_RefCountedObject(obj)
        {
            Q_ASSERT(dynamic_cast<IRefCountedObject*>(obj) != nullptr);

            if (m_RefCountedObject != nullptr) {
                m_RefCountedObject->acquire();
            }
        }

        virtual ~HoldLocker() {
            if (m_RefCountedObject != nullptr) {
                m_RefCountedObject->release();
            }
        }

    public:
        T *getArtworkMetadata() const { return m_RefCountedObject; }

    private:
        HoldLocker();
        HoldLocker(const HoldLocker&);
        HoldLocker &operator=(const HoldLocker&);

    private:
        T *m_RefCountedObject;
    };
}

#endif // IREFCOUNTEDOBJECT_H
