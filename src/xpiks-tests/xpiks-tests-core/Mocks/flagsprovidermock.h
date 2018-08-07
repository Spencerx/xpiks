#ifndef FLAGSPROVIDERMOCK_H
#define FLAGSPROVIDERMOCK_H

#include <Common/iflagsprovider.h>

namespace Mocks {
    template<typename T>
    class FlagsProviderMock: public Common::IFlagsProvider<T> {
    public:
        FlagsProviderMock(T t): m_T(t) {}

    public:
        virtual T getFlags() const override { return m_T; }

    private:
        T m_T;
    };
}

#endif // FLAGSPROVIDERMOCK_H
