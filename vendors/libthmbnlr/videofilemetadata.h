#ifndef VIDEOFILEMETADATA_H
#define VIDEOFILEMETADATA_H

#include <cstdint>
#include <string>

namespace libthmbnlr {
    struct VideoFileMetadata {
        std::string m_CodecName;
        double m_Duration;
        int64_t m_BitRate;
        double m_FrameRate;
    };
}

#endif // VIDEOFILEMETADATA_H
