#include "thumbnailcreator.h"


namespace libthmbnlr {
    ThumbnailCreator::ThumbnailCreator(const STD_STRING_TYPE &videoPath):
        m_SeekPercentage(10)
    {}

    bool ThumbnailCreator::createThumbnail(std::vector<uint8_t> &rgbBuffer, int &width, int &height) {
        (void)rgbBuffer;
        width = 0;
	    height = 0;
        return false;
    }
}
