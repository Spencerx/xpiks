#ifndef ARTWORKMETADATAMOCK
#define ARTWORKMETADATAMOCK

#include <QString>
#include <QStringList>
#include <QtGlobal>

#include "Artworks/basickeywordsmodel.h"
#include "Artworks/basicmetadatamodel.h"
#include "Artworks/imageartwork.h"
#include "Common/types.h"
#include "MetadataIO/originalmetadata.h"

namespace Mocks {
    class ArtworkMetadataMock : public Artworks::ImageArtwork {
    public:
        ArtworkMetadataMock():
            Artworks::ImageArtwork("random.jpg", 0, 0)
        {
        }

        ArtworkMetadataMock(const QString &filepath, qint64 directoryID = 0):
            Artworks::ImageArtwork(filepath, 0, directoryID)
        {
        }

        void initialize() {
            this->initAsEmpty();
        }

        void initialize(const QString &title, const QString &description, const QStringList &keywords, bool overwrite=false) {
            Q_UNUSED(overwrite);
            MetadataIO::OriginalMetadata om;
            om.m_Title = title;
            om.m_Description = description;
            om.m_Keywords = keywords;
            this->initFromOrigin(om, overwrite);
        }

        void set(const QString &title, const QString &description, const QStringList &keywords) {
            this->setTitle(title);
            this->setDescription(description);
            this->setKeywords(keywords);
            this->resetModified();
        }

        QString retrieveKeyword(int index) { return getBasicModel().retrieveKeyword(index); }
        int rowCount() const { return getBasicMetadataModel().rowCount(); }

        bool isTitleEmpty() { return getBasicMetadataModel().isTitleEmpty(); }
        bool isDescriptionEmpty() { return getBasicMetadataModel().isDescriptionEmpty(); }

        bool areKeywordsEmpty() { return getBasicModel().areKeywordsEmpty(); }

        void resetAll() { this->clearModel(); this->resetFlags(); }

        virtual void justChanged() override { /*Don't need threads*/ }

        //virtual QString getDirectory() const { return "somedirectory"; }
    };
}

#endif // ARTWORKMETADATAMOCK

