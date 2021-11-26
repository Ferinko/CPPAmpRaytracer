#pragma once

#include "Color.hpp"
#include "GeometricObject.hpp"
#include "Vec3.hpp"

namespace Smurf {

    struct Lambertian {
        Lambertian() restrict(cpu, amp) : intensity{1.0F}, color{1.0F, 1.0F, 1.0F} { }
        Lambertian(const Lambertian& other) restrict(cpu, amp) : intensity{other.intensity}, color{other.color} { }
        Lambertian& operator=(const Lambertian& other) restrict(cpu, amp) {
            if (this == &other) return *this;
            intensity = other.intensity;
            color = other.color;
            return *this;
        }
        Lambertian(float intensity, const Color& color) restrict(cpu, amp) : intensity{intensity}, color{color} { }

        Color diffuseF() const restrict(cpu, amp) {
            return intensity * color * 0.31831F; // Reciprocal PI, AMP doesn't like external constants
        }

        Color rho() const restrict(cpu, amp) {
            return intensity * color;
        }

        float intensity;
        Color color;
    };

    struct Specular {
        Specular() restrict(cpu, amp) : intensity{1.0F}, color{1.0F, 1.0F, 1.0F}, exponent{1} { }
        Specular(const Specular& other) restrict(cpu, amp) : intensity{other.intensity}, color{other.color}, exponent{other.exponent} { }
        Specular& operator=(const Specular& other) restrict(cpu, amp) {
            if (this == &other) return *this;
            intensity = other.intensity;
            color = other.color;
            exponent = other.exponent;
            return *this;
        }
        Specular(float intensity, const Color& color, float exponent) restrict(cpu, amp) : intensity{intensity}, color{color}, exponent{exponent} { }

        Color diffuseF(const Vec3<double>& normal, const Vec3<double>& origin, const Vec3<double>& direction) const restrict(amp) {
            Color result;
            float normalDotDirection = static_cast<float>(normal * direction);
            Vec3<double> reflectedDirection{-direction + 2.0 * normal * normalDotDirection};
            float reflectedDirectionDotOrigin = static_cast<float>(reflectedDirection * origin);

            if (reflectedDirectionDotOrigin > 0.0) {
                result = intensity * color * Concurrency::fast_math::powf(reflectedDirectionDotOrigin, exponent);
            }

            return result;
        }

        Color rho() const restrict(cpu, amp) {
            return {0.0F, 0.0F, 0.0F};
        }

        float intensity;
        Color color;
        float exponent;
    };

} // namespace Smurf