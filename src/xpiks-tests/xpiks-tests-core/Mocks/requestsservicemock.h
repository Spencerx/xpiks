#ifndef BROKENREQUESTSSERVICE_H
#define BROKENREQUESTSSERVICE_H

#include <QByteArray>

#include <Connectivity/irequestsservice.h>
#include <Helpers/remoteconfig.h>

namespace Mocks {
    class RequestsServiceMock: public Connectivity::IRequestsService {
    public:
        RequestsServiceMock() {}
        RequestsServiceMock(const QByteArray &response):
            m_Response(response)
        {}

        // IRequestsService interface
    public:
        virtual void receiveConfig(Helpers::RemoteConfig &config) override {
            config.setRemoteResponse(m_Response);
        }

    private:
        QByteArray m_Response;
    };
}

#endif // BROKENREQUESTSSERVICE_H
