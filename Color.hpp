#pragma once

#include "Wheels.hpp"

namespace Smurf {
    struct Color {
        // TODO - replace with numeric limits once it's properly constexpr
        static const byte ByteMax = 255;

        Color() restrict(cpu, amp) : red{0.0F}, green{0.0F}, blue{0.0F} { }
        Color(const Color& other) restrict(cpu, amp) : red{other.red}, green{other.green}, blue{other.blue} { }
        Color& operator=(const Color& other) restrict(cpu, amp) {
            if (this == &other) return *this;
            red = other.red;
            green = other.green;
            blue = other.blue;
            return *this;
        }

        Color(float red, float green, float blue) restrict(cpu, amp) : red{red}, green{green}, blue{blue} { }

        Color operator+(const Color& other) const restrict(cpu, amp) {
            return {
                red + other.red,
                green + other.green,
                blue + other.blue
            };
        }

        friend Color operator*(const Color& color, float scalar) restrict(cpu, amp) {
            return { color.red * scalar,
                     color.green * scalar,
                     color.blue * scalar
            };
        }

        friend Color operator*(float scalar, const Color& color) restrict(cpu, amp) {
            return color * scalar;
        }

        Color operator*(const Color& color) restrict(cpu, amp) {
            return {
                red * color.red,
                green * color.green,
                blue * color.blue
            };
        }

        Color& operator+=(const Color& other) restrict(cpu, amp) {
            red += other.red;
            green += other.green;
            blue += other.blue;
            return *this;
        }

        Color& operator/=(int scalar) restrict(cpu, amp) {
            red /= scalar;
            green /= scalar;
            blue /= scalar;
            return *this;
        }
        
        Color& operator*=(float scalar) restrict(cpu, amp) {
            red *= scalar;
            green *= scalar;
            blue *= scalar;
            return *this;
        }

        bool operator==(const Color& other) const restrict(cpu, amp) {
            return red == other.red && green == other.green && blue == other.blue;
        }

        float red;
        float green;
        float blue;
    };

    namespace Colors {
        const Color Black = { 0.0F, 0.0F, 0.0F };
        const Color White = { 1.0F, 1.0F, 1.0F };
        const Color Red = { 1.0F, 0.0F, 0.0F };
        const Color Green = { 0.0F, 1.0F, 0.0F };
        const Color Blue = { 0.0F, 0.0F, 1.0F };
    } // namespace Colors
} // namespace Smurf