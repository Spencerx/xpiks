#ifndef FTPCOORDINATOR_H
#define FTPCOORDINATOR_H

#include <cstddef>
#include <memory>
#include <vector>

#include <QAtomicInt>
#include <QMutex>
#include <QObject>
#include <QSemaphore>
#include <QString>

#include "Artworks/artworkssnapshot.h"
#include "Connectivity/iftpcoordinator.h"

namespace Encryption {
    class SecretsManager;
}

namespace Models {
    class SettingsModel;
    class UploadInfo;
}

namespace libxpks {
    namespace net {
        class FtpCoordinator :
                public QObject,
                public Connectivity::IFtpCoordinator
        {
            Q_OBJECT
        public:
            explicit FtpCoordinator(Encryption::SecretsManager &secretsManager,
                                    Models::SettingsModel &settings,
                                    QObject *parent = 0);

        public:
            // IFTPCOORDINATOR
            virtual void uploadArtworks(const Artworks::ArtworksSnapshot &artworksToUpload,
                                        std::vector<std::shared_ptr<Models::UploadInfo> > &uploadInfos) override;
            virtual void cancelUpload() override;

        signals:
            void uploadStarted();
            void cancelAll();
            void uploadFinished(bool anyError);
            void overallProgressChanged(double percentDone);
            void transferFailed(const QString &filepath, const QString &host);

        private slots:
            void workerProgressChanged(double oldPercents, double newPercents);
            void workerFinished(bool anyErrors);

        private:
            void initUpload(size_t uploadBatchesCount);
            void finalizeUpload();

        private:
            QMutex m_WorkerMutex;
            QSemaphore m_UploadSemaphore;
            Encryption::SecretsManager &m_SecretsManager;
            Models::SettingsModel &m_Settings;
            double m_OverallProgress;
            QAtomicInt m_FinishedWorkersCount;
            volatile size_t m_AllWorkersCount;
            volatile bool m_AnyFailed;
        };
    }
}

#endif // FTPCOORDINATOR_H
