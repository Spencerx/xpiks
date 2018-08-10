#ifndef IMICROSTOCKAPICLIENTS_H
#define IMICROSTOCKAPICLIENTS_H

#include <memory>
#include "microstockenums.h"

namespace Microstocks {
    class IMicrostockAPIClient;

    class IMicrostockAPIClients {
    public:
        virtual ~IMicrostockAPIClients() {}

        virtual std::shared_ptr<IMicrostockAPIClient> getClient(MicrostockType type) = 0;
    };
}

#endif // IMICROSTOCKAPICLIENTS_H
