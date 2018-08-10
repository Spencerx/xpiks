#ifndef MICROSTOCKCLIENTSMOCK_H
#define MICROSTOCKCLIENTSMOCK_H

#include <Microstocks/imicrostockapiclients.h>

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
