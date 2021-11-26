#pragma once

#ifdef USE_AMP
#include <amp_math.h>
#endif

namespace Smurf {
    template <typename T>
    struct Vec3 {
        Vec3() restrict(cpu, amp) { }
        Vec3(T x, T y, T z) restrict(cpu, amp) : x{x}, y{y}, z{z} { }

        inline friend Vec3<T> operator+(const Vec3<T>& lhs, const Vec3<T>& rhs) restrict(cpu, amp) {
            return { lhs.x + rhs.x,
                     lhs.y + rhs.y,
                     lhs.z + rhs.z };
        }

        inline Vec3<T> operator+=(const Vec3<T>& rhs) restrict(cpu, amp) {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }

        inline friend Vec3<T> operator-(const Vec3<T>& lhs, const Vec3<T>& rhs) restrict(cpu, amp) {
            return { lhs.x - rhs.x,
                     lhs.y - rhs.y,
                     lhs.z - rhs.z };
        }

        inline Vec3<T> operator-=(const Vec3<T>& rhs) restrict(cpu, amp) {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }

        inline friend Vec3<T> operator-(const Vec3<T>& vec) restrict(cpu, amp) {
            return {
                -vec.x,
                -vec.y,
                -vec.z
            };
        }

        inline friend T operator*(const Vec3<T>& lhs, const Vec3<T>& rhs) restrict(cpu, amp) {
            return lhs.x * rhs.x +
                   lhs.y * rhs.y +
                   lhs.z * rhs.z;
        }

        inline friend Vec3<T> operator*(const Vec3<T>& lhs, double scalar) restrict(cpu, amp) {
            return { lhs.x * scalar,
                     lhs.y * scalar,
                     lhs.z * scalar };
        }

        inline friend Vec3<T> operator*(double scalar, const Vec3<T>& rhs) restrict(cpu, amp) {
            return rhs * scalar;
        }

        inline void normalize() restrict(cpu, amp) {
            double len = length();
            x /= len; y /= len; z /= len;
        }

        inline Vec3<T> normalizeAndReturn() restrict(cpu, amp) {
            double len = length();
            x /= len; y /= len; z /= len;
            return *this;
        }

        inline double length() const restrict(cpu) {
            return sqrt(x * x + y * y + z * z);
        }

        inline double lengthSquared() const restrict(cpu, amp) {
            return x * x + y * y + z * z;
        }

        inline Vec3<T> crossProduct(const Vec3<T>& other) const restrict(cpu, amp) {
            return { y * other.z - z * other.y,
                     z * other.x - x * other.z,
                     x * other.y - y * other.x };
        }

        #ifdef USE_AMP

        inline T distance(const Vec3<T>& other) const restrict(amp) {
            return {
                Concurrency::fast_math::sqrtf(
                    static_cast<float>(
                        (x - other.x) * (x - other.x) +
                        (y - other.y) * (y - other.y) +
                        (z - other.z) * (z - other.z)
                    )
                )
            };
        }

        inline double length() const restrict(amp) {
            return Concurrency::fast_math::sqrtf(static_cast<float>(x * x + y * y + z * z));
        }
       
        #endif
        T x, y, z;
    };
} // namespace Smurf