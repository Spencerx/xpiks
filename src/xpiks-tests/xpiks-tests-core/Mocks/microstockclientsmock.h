#ifndef MICROSTOCKCLIENTSMOCK_H
#define MICROSTOCKCLIENTSMOCK_H

#include <memory>

#include <QtGlobal>

#include "Microstocks/imicrostockapiclients.h"
#include "Microstocks/microstockenums.h"

namespace Microstocks {
    class IMicrostockAPIClient;
}

namespace Mocks {
    class MicrostockAPIClientsMock : public Microstocks::IMicrostockAPIClients {
        // IMicrostockAPIClients interface
    public:
        virtual std::shared_ptr<Microstocks::IMicrostockAPIClient> getClient(Microstocks::MicrostockType type) override {
            Q_UNUSED(type);
            return std::shared_ptr<Microstocks::IMicrostockAPIClient>();
        }
    };
}

#endif // MICROSTOCKCLIENTSMOCK_H
