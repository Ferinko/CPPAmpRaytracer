#pragma once

#include "Color.hpp"
#include "Vec3.hpp"

namespace Smurf {

    class DirectionalLight {
    public:
        DirectionalLight() restrict(cpu, amp) : color{1.0F, 1.0F, 1.0F}, radianceScale{1.0F}, whence{0.0F, 1.0F, 0.0F} { }
        DirectionalLight(const DirectionalLight& other) restrict(cpu, amp) : color{other.color}, radianceScale{other.radianceScale}, whence{other.whence} { }
        DirectionalLight(Color color, float radianceScale, Vec3<double> whence) restrict(cpu, amp) : color{color}, radianceScale{radianceScale} {
            setWhence(whence);
        }

        void setWhence(double x, double y, double z) restrict(cpu, amp) {
            setWhence({ x, y, z });
        }

        void setWhence(const Vec3<double>& whence) restrict(cpu, amp) {
            this->whence = whence;
            this->whence.normalize();
        }

        const Vec3<double>& getDirection() const restrict(cpu, amp) {
            return whence;
        }

        Color getRadiance() const restrict(cpu, amp) {
            return radianceScale * color;
        }

        Color color;
        float radianceScale;
    private:
        Vec3<double> whence;
    };

    class PointLight {
    public:
        PointLight() restrict(cpu, amp) : color{1.0F, 1.0F, 1.0F}, radianceScale{1.0F}, location{0.0F, 0.0F, 0.0F} { }
        PointLight(const Color& color, float radianceScale, const Vec3<double>& location) : color{color}, radianceScale{radianceScale}, location{location} { }
        PointLight(const PointLight& other) restrict(cpu, amp) : color{other.color}, radianceScale{other.radianceScale}, location{other.location} { }

        void setLocation(double x, double y, double z) restrict(cpu, amp) {
            setLocation({ x, y, z });
        }

        void setLocation(Vec3<double> location) restrict(cpu, amp) {
            this->location = location;
        }

        Vec3<double> getDirection(const Vec3<double> hitPoint) const restrict(cpu, amp) {
            return (location - hitPoint).normalizeAndReturn();
        }

        Color getRadiance() const restrict(cpu, amp) {
            return radianceScale * color;
        }

        Color color;
        float radianceScale;
        Vec3<double> location;
    };

    class AmbientLight {
    public:
        AmbientLight() restrict(cpu, amp) : color{1.0F, 1.0F, 1.0F}, radianceScale{1.0F} { }
        AmbientLight(const AmbientLight& other) restrict(cpu, amp) : color{other.color}, radianceScale{other.radianceScale} { }

        Vec3<double> getDirection() const restrict(cpu, amp) {
            return { 0.0, 0.0, 0.0 };
        }

        Color getRadiance() const restrict(cpu, amp) {
            return radianceScale * color;
        }

        Color color;
        float radianceScale;
    };

} // namespace Smurf