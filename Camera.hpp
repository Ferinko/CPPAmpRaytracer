#pragma once

#include "Vec3.hpp"

#include <array>

namespace Smurf {
    class Camera {
    public:
        Camera() restrict(cpu, amp) : eyePos{0, 0, 0},
                                      lookAt{0, 0, 0},
                                      up{0, 1, 0},
                                      viewPlaneDistance{100} {
            initializeCameraCoordSystem();
        }

        Camera(Vec3<double> eyePos, Vec3<double> lookAt, Vec3<double> up, float viewPlaneDistance) restrict(cpu, amp) :
            eyePos{eyePos},
            lookAt{lookAt},
            up{up},
            viewPlaneDistance{viewPlaneDistance} {
            initializeCameraCoordSystem();
        }

        void initializeCameraCoordSystem() restrict(cpu, amp) {
            bool skip = false;
            if (eyePos.x == lookAt.x &&
                eyePos.z == lookAt.z) {
                if (eyePos.y > lookAt.y) {
                    // Camera tilted 90degrees down
                    cameraX = { 0, 0, 1 };
                    cameraY = { 1, 0, 0 };
                    cameraZ = { 0, 1, 0 };
                    skip = true;
                } else if (eyePos.y < lookAt.y) {
                    // Camera tilted 90degrees up
                    cameraX = { 1, 0, 0 };
                    cameraY = { 0, 0, 1 };
                    cameraZ = { 0, -1, 0 };
                    skip = true;
                }
            }
            if (!skip) {
                cameraZ = eyePos - lookAt;
                cameraZ.normalize();
                cameraX = up.crossProduct(cameraZ);
                cameraX.normalize();
                cameraY = cameraZ.crossProduct(cameraX);
            }
        }

        Vec3<double> inferRayDirection(const Vec2<double>& pixel) const restrict(cpu, amp) {
            auto direction = pixel.x * cameraX + pixel.y * cameraY - viewPlaneDistance * cameraZ;
            direction.normalize();
            return direction;
        }

        float getVPDistance() const restrict(cpu, amp) {
            return viewPlaneDistance;
        }

        const Vec3<double>& getCameraAxisX() const restrict(cpu, amp) {
            return {cameraX};
        }

        const Vec3<double>& getCameraAxisY() const restrict(cpu, amp) {
            return {cameraY};
        }

        const Vec3<double>& getCameraAxisZ() const restrict(cpu, amp) {
            return {cameraZ};
        }

        const Vec3<double>& getEye() const restrict(cpu, amp) {
            return eyePos;
        }

        const Vec3<double>& getLookAt() const restrict(cpu, amp) {
            return lookAt;
        }

        const Vec3<double>& getUp() const restrict(cpu, amp) {
            return up;
        }

    private:
        Vec3<double> cameraX, cameraY, cameraZ;
        Vec3<double> eyePos;
        Vec3<double> lookAt;
        Vec3<double> up;
        float viewPlaneDistance;
    };
} // namespace Smurf