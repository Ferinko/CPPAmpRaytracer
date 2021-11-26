#pragma once

#include "Wheels.hpp"
#include "Pixel.hpp"
#include "_DIBHeader.hpp"
#include "_BitmapFileHeader.hpp"

#include <boost/detail/endian.hpp>

#include <intrin.h>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <tuple>
#include <functional>
#include <memory>
#include <array>
#include <vector>

namespace Smurf {
    namespace FileFormat {
        class Bitmap {
        public:
            enum class Origin { BottomLeft, TopLeft };
            enum ColorDepth { bpp24 = 24, bpp36 = 36 };

            Bitmap() : _bitmapFileHeader{}, _dibHeader{} { }
            
            Bitmap(int hRes, int vRes, 
                   Origin origin   = Origin::BottomLeft,
                   word colorDepth = ColorDepth::bpp24) {
                _dibHeader.width = hRes;
                _dibHeader.height = vRes;
                int32_t imageSize = hRes * vRes * Utils::storedInNBytes(_dibHeader.colorDepth) + getTotalPaddingSize();
                _bitmapFileHeader.fileSize += imageSize;
                _dibHeader.imageSize = imageSize;
                image.reserve(sizeof(byte) * abs(imageSize));
            }

            Bitmap(int hRes, int vRes, std::vector<Pixel> data,
                   Origin origin   = Origin::BottomLeft,
                   word colorDepth = ColorDepth::bpp24) :
                   Bitmap{hRes, vRes, origin, colorDepth} {
                image = data;
            }

            friend std::ostream& operator<<(std::ostream& os, const Bitmap& bitmap) {
                os << bitmap._bitmapFileHeader << bitmap._dibHeader;
                bitmap.writeData(os);
                return os;
            }

        private:
            int getRowPadding() const {
                // Padding so that rows are divisible by 4
                return (sizeof(dword) - (_dibHeader.width * Utils::storedInNBytes(_dibHeader.colorDepth))) % sizeof(dword);
            }

            int getTotalPaddingSize() const {
                return getRowPadding() * _dibHeader.height;
            }

            void writeData(std::ostream& os) const {
                auto numPad = getRowPadding();
                byte pad[] = { 0x00, 0x00, 0x00 };
                for (int row = 0; row < abs(_dibHeader.height); ++row) {
                    for (int col = 0; col < _dibHeader.width; ++col) {
                        os << image[row * _dibHeader.width + col];
                    }
                    os.write(reinterpret_cast<const char*>(pad), numPad);
                }
            }
        private:
            _BitmapFileHeader _bitmapFileHeader;
            _DIBHeader _dibHeader;
            // TODO - C++14 change to dynarray<>
            std::vector<Pixel> image;
        };
    } // namespace FileFormat
} // namespace Smurf