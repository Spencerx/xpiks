#ifndef ARTWORKSREPOSITORYMOCK_H
#define ARTWORKSREPOSITORYMOCK_H

#include <QVector>
#include <QPair>
#include "../../xpiks-qt/Models/artworksrepository.h"
#include "../../xpiks-qt/Models/recentdirectoriesmodel.h"

namespace Mocks {
    class ArtworksRepositoryMock : public Models::ArtworksRepository {
    public:
        ArtworksRepositoryMock(Common::ISystemEnvironment &environment):
            Models::ArtworksRepository(m_RecentDirectories),
            m_RecentDirectories(environment)
        {
            m_RecentDirectories.initialize();
        }

        void removeFileAndEmitSignal() {
            insertIntoUnavailable(*getFilesSet().begin());
            emit filesUnavailable();
        }

        void removeVectorAndEmitSignal() {
            QString vector = *getFilesSet().begin();
            vector.replace(".jpg", ".eps");
            insertIntoUnavailable(vector);
            emit filesUnavailable();
        }

        void setWasAddedAsFullDirectory(int index) { setFullDirectory(index); }
        void unsetWasAddedAsFullDirectory(int index) { unsetFullDirectory(index); }

    private:
        Models::RecentDirectoriesModel m_RecentDirectories;
    };
}

#endif // ARTWORKSREPOSITORYMOCK_H

