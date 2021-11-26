#pragma once

#include "Vec3.hpp"

namespace Smurf {
    struct Ray {
        Vec3<double> origin;
        Vec3<double> direction;

        Ray() restrict(cpu, amp) { }
        Ray(const Vec3<double>& origin, const Vec3<double>& direction) restrict(cpu, amp) : origin{origin},
                                                                                            direction{direction} { }
    };
} // namespace Smurf