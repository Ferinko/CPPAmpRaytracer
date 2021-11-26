#pragma once

#ifdef USE_AMP
#include <amp_math.h>
#endif

namespace Smurf {
    template <typename T>
    struct Vec2 {
        Vec2() restrict(cpu, amp) { }
        Vec2(T x, T y) restrict(cpu, amp) : x{ x }, y{ y } { }

        inline friend Vec2<T> operator+(const Vec2<T>& lhs, const Vec2<T>& rhs) restrict(cpu, amp) {
            return Vec2(lhs.x + rhs.x,
                        lhs.y + lhs.y);
        }

        inline Vec2<T> operator+=(const Vec2<T>& rhs) restrict(cpu, amp) {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        inline friend Vec2<T> operator-(const Vec2<T>& lhs, const Vec2<T>& rhs) restrict(cpu, amp) {
            return Vec2<T>(lhs.x - rhs.x,
                           lhs.y - rhs.y);
        }

        inline Vec2<T> operator-=(const Vec2<T>& rhs) restrict(cpu, amp) {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        inline friend T operator*(const Vec2<T>& lhs, const Vec2<T>& rhs) restrict(cpu, amp) {
            return lhs.x * rhs.x +
                   lhs.y * rhs.y;
        }

        inline friend Vec2<T> operator*(const Vec2<T>& lhs, double scalar) restrict(cpu, amp) {
            return Vec2<T>(lhs.x * scalar,
                           lhs.y * scalar);
        }

        inline friend Vec2<T> operator*(double scalar, const Vec2<T>& rhs) restrict(cpu, amp) {
            return rhs * scalar;
        }

        inline void normalize() restrict(cpu, amp) {
            double len = length();
            x /= len; y /= len;
        }

        inline double length() const restrict(cpu) {
            return sqrt(x * x + y * y);
        }

        inline double lengthSquared() const restrict(cpu, amp) {
            return x * x + y * y;
        }

        #ifdef USE_AMP
        
        inline double length() const restrict(amp) {
            return Concurrency::fast_math::sqrtf(static_cast<float>(x * x + y * y));
        }

        #endif

        T x, y;
    };
} // namespace Smurf