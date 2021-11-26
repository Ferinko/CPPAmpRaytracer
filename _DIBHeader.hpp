#pragma once

#include "Wheels.hpp"

#include <boost/detail/endian.hpp>

#include <tuple>

namespace Smurf {
    struct _DIBHeader {
        enum CompressionMethod {
            bi_rgb = 0,
            bi_rle8 = 1,
            bi_rle4 = 2,
            bi_bitfields = 3,
            bi_jpeg = 4,
            bi_png = 5,
            bi_alphabitfields = 6
        };

        _DIBHeader() : width{0},
                       height{0},
                       colorDepth{24},
                       compression{CompressionMethod::bi_rgb},
                       imageSize{0},
                       hResPixelPerMeter{2835},
                       vResPixelPerMeter{2835},
                       numColorsUsed{0},
                       numImportantColors{0} { }

        static const dword headerSize = 40;
        int32_t width;
        int32_t height;
        static const word numColorPlanes = 1;
        word colorDepth;
        dword compression;
        dword imageSize;
        dword hResPixelPerMeter;
        dword vResPixelPerMeter;
        dword numColorsUsed;
        dword numImportantColors;

        friend std::ostream& operator<<(std::ostream& os, const _DIBHeader& _dibHeader) {
            std::tuple<const dword&, int32_t&, int32_t&, const word&, word&, dword&, dword&, dword&, dword&, dword&, dword&> fields(
                std::tie(
                    _dibHeader.headerSize, const_cast<int32_t&>(_dibHeader.width), const_cast<int32_t&>(_dibHeader.height),
                    _dibHeader.numColorPlanes, const_cast<word&>(_dibHeader.colorDepth), const_cast<dword&>(_dibHeader.compression),
                    const_cast<dword&>(_dibHeader.imageSize), const_cast<dword&>(_dibHeader.hResPixelPerMeter),
                    const_cast<dword&>(_dibHeader.vResPixelPerMeter), const_cast<dword&>(_dibHeader.numColorsUsed),
                    const_cast<dword&>(_dibHeader.numImportantColors)));

            #ifndef BOOST_LITTLE_ENDIAN
            Smurf::CompileTime::foreachElemInTuple(fields, Utils::_SwapEndianOutputRefForward());
            #endif
            Smurf::CompileTime::foreachElemInTuple(fields, Utils::_WriteToStreamForward(os));
            return os;
        }
    };
} // namespace Smurf