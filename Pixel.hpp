#pragma once

#include "Color.hpp"

#include <array>

#ifdef USE_AMP
#undef max
#endif

namespace Smurf {
    struct Pixel {
        Pixel() : data{ {0x00, 0x00, 0x00} } { }
        Pixel(byte blue, byte green, byte red) : data{ {blue, green, red} } { }

        explicit Pixel(const Color& color) {
            // Scale the color appropriately
            float maxColor = std::max(color.red, std::max(color.green, color.blue));
            if (maxColor > 1.0F) {
                data[0] = static_cast<byte>((color.blue / maxColor) * Color::ByteMax);
                data[1] = static_cast<byte>((color.green / maxColor) * Color::ByteMax);
                data[2] = static_cast<byte>((color.red / maxColor) * Color::ByteMax);
            } else {
                data[0] = static_cast<byte>(color.blue * Color::ByteMax);
                data[1] = static_cast<byte>(color.green * Color::ByteMax);
                data[2] = static_cast<byte>(color.red * Color::ByteMax);
            }
        }

        friend std::ostream& operator<<(std::ostream& os, const Pixel& pixel) {
            Utils::streamWrite(os, pixel);
            return os;
        }

        std::array<byte, 3> data;
    };
} // namespace Smurf