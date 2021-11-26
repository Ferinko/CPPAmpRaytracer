#pragma once

#include "Material.hpp"
#include "Vec3.hpp"
#include "Ray.hpp"
#include "Color.hpp"

#include <boost\optional.hpp>

#ifdef USE_AMP
#include <amp_math.h>
#endif

namespace Smurf {
    enum ActiveMaterial { ActiveMatte, ActiveGlossy };

    struct RayHit {
        RayHit() : depth{0}, hitPoint{0.0, 0.0, 0.0}, tMin{0.0}  { }
        RayHit(double tMin) : tMin{tMin} { }
        RayHit(double tMin, Vec3<double> hitPoint, int depth) : depth{depth}, hitPoint{hitPoint}, tMin{tMin} { }

        int depth;
        Vec3<double> hitPoint;
        double tMin;
    };

    struct GeometricObject {
        GeometricObject() : color{0.7F, 0.65F, 1.0F} { }
        GeometricObject(Color color) : color{color} { }
        GeometricObject(Color color, Matte material) : color{ color }, matte{ material }, active{ ActiveMaterial::ActiveMatte } { }
        GeometricObject(Color color, Glossy material) : color{ color }, glossy{ material }, active{ ActiveMaterial::ActiveGlossy } { }
        virtual boost::optional<RayHit> onRayCast(const Ray& ray) = 0;
        virtual ~GeometricObject() { }
        // Probably replace this later
        Color color;
        Matte matte;
        Glossy glossy;
        ActiveMaterial active;
    };

    class Plane : public GeometricObject {
    public:
        Plane() : point{0.0, 0.0, 0.0}, normal{0.0, 1.0, 0.0} { }
        Plane(const Vec3<double>& point, const Vec3<double>& normal, Color color) : GeometricObject{color},
                                                                                    point{point},
                                                                                    normal{normal} { }
        Plane(const Vec3<double>& point, const Vec3<double>& normal, Color color, Matte matte) : GeometricObject{ color, matte },
                                                                                                 point{ point },
                                                                                                 normal{ normal } { }
        Plane(const Vec3<double>& point, const Vec3<double>& normal, Color color, Glossy glossy) : GeometricObject{ color, glossy },
                                                                                                   point{ point },
                                                                                                   normal{ normal } { }
        boost::optional<RayHit> onRayCast(const Ray& ray) override {
            auto t = (point - ray.origin) * normal / (ray.direction * normal);
            if (t > 0) {
                RayHit hit(t);
                return boost::optional<RayHit>(hit);
            }
            // Didn't hit
            return boost::optional<RayHit>();
        }
        #ifdef USE_AMP
        const Vec3<double>& getPoint() {
            return point;
        }
        const Vec3<double>& getNormal() {
            return normal;
        }
        #endif
    private:
        Vec3<double> point;
        Vec3<double> normal;
    };

    class Sphere : public GeometricObject {
    public:
        Sphere() : center{0.0, 0.0, 0.0}, radius{1.0} { }
        Sphere(const Vec3<double>& center, double radius) : center{center},
                                                            radius{radius} { }
        Sphere(const Vec3<double>& center, double radius, Matte material) : GeometricObject{ {0.0F, 0.0F, 0.0F}, material },
                                                                            center{ center },
                                                                            radius{ radius } { }
        Sphere(const Vec3<double>& center, double radius, Glossy material) : GeometricObject{ { 0.0F, 0.0F, 0.0F }, material },
                                                                            center{ center },
                                                                            radius{ radius } { }
        boost::optional<RayHit> onRayCast(const Ray& ray) override {
            auto temp = ray.origin - center;
            auto a = ray.direction * ray.direction;
            auto b = ray.direction * (2.0 * temp);
            auto c = temp * temp - radius * radius;
            auto discriminant = b * b - (4.0 * a * c);

            // Didn't hit
            if (discriminant < 0.0) return boost::optional<RayHit>();

            auto e = sqrt(discriminant);
            auto quadraticDenominator = 2.0 * a;

            auto finalize = [](double tMin) -> boost::optional<RayHit> {
                RayHit hit(tMin);
                return boost::optional<RayHit>(hit);
            };

            auto t = (-b - e) / quadraticDenominator;
            if (t > 0) return finalize(t);

            t = (-b + e) / quadraticDenominator;
            if (t > 0) return finalize(t);

            // Didn't hit
            return boost::optional<RayHit>();
        }
        #ifdef USE_AMP
        const Vec3<double>& getCenter() {
            return center;
        }
        double getRadius() {
            return radius;
        }
        #endif
    private:
        Vec3<double> center;
        double radius;
    };

    class Rectangle : public GeometricObject {
    public:
        Rectangle() : point{0.0, 0.0, 0.0}, a{5.0, 0.0, 0.0}, b{0.0, -5.0, 0.0}, normal{0.0, 0.0, 1.0} { }
        Rectangle(Vec3<double> point, Vec3<double> a, Vec3<double> b, Vec3<double> normal) : point{point}, a{a}, b{b}, normal{normal} { }
        Rectangle(Vec3<double> point, Vec3<double> a, Vec3<double> b, Vec3<double> normal, Matte material) : GeometricObject{ { 0.0F, 0.0F, 0.0F }, material }, point{ point }, a{ a }, b{ b }, normal{ normal } { }
        Rectangle(Vec3<double> point, Vec3<double> a, Vec3<double> b, Vec3<double> normal, Glossy material) : GeometricObject{ { 0.0F, 0.0F, 0.0F }, material }, point{ point }, a{ a }, b{ b }, normal{ normal } { }

        boost::optional<RayHit> onRayCast(const Ray& ray) override {
            double t = (point - ray.origin) * normal / (ray.direction * normal);
            if (t <= 0) return {};

            auto temp = ray.origin + t * ray.direction;
            auto tempDir = temp - point;

            auto tempDirDotSide = tempDir * a;
            if (tempDirDotSide > a.lengthSquared() || tempDirDotSide < 0.0) {
                return {};
            }

            tempDirDotSide = tempDir * b;
            if (tempDirDotSide > b.lengthSquared() || tempDirDotSide < 0.0) {
                return {};
            }

            return {{t}};
        }
        #ifdef USE_AMP
        const Vec3<double>& getPoint() {
            return point;
        }
        const Vec3<double>& getA() {
            return a;
        }
        const Vec3<double>& getB() {
            return b;
        }
        const Vec3<double>& getNormal() {
            return normal;
        }
        #endif
    private:
        Vec3<double> point;
        Vec3<double> a, b;
        Vec3<double> normal;
    };
    #ifdef USE_AMP

    struct g_Plane {
    public:
        g_Plane() restrict(cpu, amp) : point{ 0.0, 0.0, 0.0 }, normal{ 0.0, 1.0, 0.0 }, matte{}, active{ ActiveMaterial::ActiveMatte } { }
        g_Plane(const Vec3<double>& point, const Vec3<double>& normal, const Matte& material) restrict(cpu, amp) : point{ point },
                                                                                                                   normal{ normal },
                                                                                                                   matte{ material },
                                                                                                                   active{ ActiveMaterial::ActiveMatte } { }
        g_Plane(const Vec3<double>& point, const Vec3<double>& normal, const Glossy& material) restrict(cpu, amp) : point{ point },
                                                                                                                    normal{ normal },
                                                                                                                    glossy{ material },
                                                                                                                    active{ ActiveMaterial::ActiveGlossy } { }
        Vec3<double> point;
        Vec3<double> normal;
        ActiveMaterial active;
        Matte matte;
        Glossy glossy;
    };

    struct g_Sphere {
        g_Sphere() restrict(amp) : center{ 0.0, 0.0, 0.0 }, radius{ 1.0 }, matte{}, active{ ActiveMaterial::ActiveMatte } { }
        g_Sphere(const Vec3<double>& center, double radius, const Matte& material) restrict(cpu, amp) : center{center},
                                                                                                        radius{radius},
                                                                                                        matte{material},
                                                                                                        active{ ActiveMaterial::ActiveMatte } { }
        g_Sphere(const Vec3<double>& center, double radius, const Glossy& material) restrict(cpu, amp) : center{ center },
                                                                                                        radius{ radius },
                                                                                                        glossy{ material },
                                                                                                        active{ ActiveMaterial::ActiveGlossy } { }
        ActiveMaterial active;
        Vec3<double> center;
        double radius;
        Matte matte;
        Glossy glossy;
    };

    struct g_Rectangle {
        g_Rectangle() restrict(cpu, amp) : point{0.0, 0.0, 0.0}, a{5.0, 0.0, 0.0}, b{0.0, -5.0, 0.0}, normal{0.0, 0.0, 1.0} { }
        g_Rectangle(const Vec3<double>& point, const Vec3<double>& a, const Vec3<double>& b, const Vec3<double>& normal, const Matte& matte) restrict(cpu, amp) : point{ point }, a{ a }, b{ b }, normal{ normal }, matte{ matte }, active{ ActiveMaterial::ActiveMatte } { }
        g_Rectangle(const Vec3<double>& point, const Vec3<double>& a, const Vec3<double>& b, const Vec3<double>& normal, const Glossy& glossy) restrict(cpu, amp) : point{ point }, a{ a }, b{ b }, normal{ normal }, glossy{ glossy }, active{ ActiveMaterial::ActiveGlossy } { }
        g_Rectangle(const g_Rectangle& other) restrict(cpu, amp) : point{ other.point }, a{ other.a }, b{ other.b }, normal{ other.normal }, matte{ other.matte }, glossy{ other.glossy }, active{ other.active } { }
        g_Rectangle(g_Rectangle&& other) restrict(cpu, amp) : point{static_cast<Vec3<double>&&>(other.point)},
                                                              a{static_cast<Vec3<double>&&>(other.a)},
                                                              b{static_cast<Vec3<double>&&>(other.b)},
                                                              normal{static_cast<Vec3<double>&&>(other.normal)},
                                                              matte{static_cast<Matte&&>(other.matte)},
                                                              glossy{ static_cast<Glossy&&>(other.glossy)},
                                                              active{ static_cast<ActiveMaterial&&>(other.active) } { }
        g_Rectangle& operator=(const g_Rectangle& other) restrict(cpu, amp) {
            point = other.point;
            a = other.a;
            b = other.b;
            normal = other.normal;
            active = other.active;
            return *this;
        }

        ActiveMaterial active;
        Vec3<double> point;
        Vec3<double> a, b;
        Vec3<double> normal;
        Matte matte;
        Glossy glossy;
    };

    struct g_RayHit {
        g_RayHit() restrict(amp) : hasHit{false} { }
        g_RayHit(double tMin, const Vec3<double>& normal, ActiveMaterial active) restrict(amp) : tMin{ tMin }, normal{ normal }, hasHit{ true }, active{ active } { }

        operator bool() const restrict(amp) {
            return hasHit;
        }

        Ray ray;
        int depth;
        ActiveMaterial active;
        Vec3<double> hitPoint;
        Vec3<double> normal;
        Matte matte;
        Glossy glossy;
        double tMin;
        bool hasHit;
    };

    struct g_ShadowRayHit {
        g_ShadowRayHit() restrict(amp) : hasHit{false} { }
        g_ShadowRayHit(float t) restrict(amp) : t{t}, hasHit{true} { }

        operator bool() const restrict(amp) {
            return hasHit;
        }

        float t;
        bool hasHit;
    };

    namespace OnRayCastAspect {
        static float const GetEpsilon() restrict(cpu, amp) {
            return 0.0001F;
        }

        g_RayHit onRayCast(const g_Plane& plane, const Ray& ray) restrict(amp) {
            auto t = (plane.point - ray.origin) * plane.normal / (ray.direction * plane.normal);
            if (t > GetEpsilon()) {
                g_RayHit hit{t, plane.normal, plane.active};
                return hit;
            }
            // Didn't hit
            return {};
        }

        g_ShadowRayHit onShadowRayCast(const g_Plane& plane, const Ray& ray) restrict(amp) {
            auto t = static_cast<float>((plane.point - ray.origin) * plane.normal / (ray.direction * plane.normal));
            if (t > GetEpsilon()) {
                return {t};
            }
            return {};
        }

        g_RayHit onRayCast(const g_Sphere& sphere, const Ray& ray) restrict(amp) {
            auto temp = ray.origin - sphere.center;
            auto a = ray.direction * ray.direction;
            auto b = ray.direction * (2.0 * temp);
            auto c = temp * temp - sphere.radius * sphere.radius;
            auto discriminant = b * b - (4.0 * a * c);

            // Didn't hit
            if (discriminant < 0.0) return {};

            auto e = Concurrency::fast_math::sqrt(static_cast<float>(discriminant));
            auto quadraticDenominator = 2.0 * a;

            auto finalize = [](double tMin, const Vec3<double>& normal, const g_Sphere& sphere) restrict(amp) {
                return g_RayHit{tMin, normal, sphere.active};
            };

            auto t = (-b - e) / quadraticDenominator;
            if (t > GetEpsilon()) return finalize(t, (temp + t * ray.direction).normalizeAndReturn(), sphere);

            t = (-b + e) / quadraticDenominator;
            if (t > GetEpsilon()) return finalize(t, (temp + t * ray.direction).normalizeAndReturn(), sphere);

            // Didn't hit
            return {};
        }

        g_ShadowRayHit onShadowRayCast(const g_Sphere& sphere, const Ray& ray) restrict(amp) {
            auto temp = ray.origin - sphere.center;
            auto a = ray.direction * ray.direction;
            auto b = ray.direction * (2.0 * temp);
            auto c = temp * temp - sphere.radius * sphere.radius;
            auto discriminant = b * b - (4.0 * a * c);

            // Didn't hit
            if (discriminant < 0.0) return {};

            auto e = Concurrency::fast_math::sqrt(static_cast<float>(discriminant));
            auto quadraticDenominator = 2.0 * a;

            auto t = static_cast<float>((-b - e) / quadraticDenominator);
            if (t > GetEpsilon()) return {t};

            t = static_cast<float>((-b + e) / quadraticDenominator);
            if (t > GetEpsilon()) return {t};

            // Didn't hit
            return {};
        }

        g_RayHit onRayCast(const g_Rectangle& rect, const Ray& ray) restrict(amp) {
            double t = (rect.point - ray.origin) * rect.normal / (ray.direction * rect.normal);
            
            // Didn't hit
            if (t <= 0) return {};

            auto temp = ray.origin + t * ray.direction;
            auto tempDir = temp - rect.point;

            auto tempDirDotSide = tempDir * rect.a;
            if (tempDirDotSide > rect.a.lengthSquared() || tempDirDotSide < 0.0) {
                // Didn't hit
                return {};
            }

            tempDirDotSide = tempDir * rect.b;
            if (tempDirDotSide > rect.b.lengthSquared() || tempDirDotSide < 0.0) {
                // Didn't hit
                return {};
            }

            return {t, rect.normal, rect.active};
        }

        g_ShadowRayHit onShadowRayCast(const g_Rectangle& rect, const Ray& ray) restrict(amp) {
            double t = (rect.point - ray.origin) * rect.normal / (ray.direction * rect.normal);

            // Didn't hit
            if (t <= 0) return {};

            auto temp = ray.origin + t * ray.direction;
            auto tempDir = temp - rect.point;

            auto tempDirDotSide = tempDir * rect.a;
            if (tempDirDotSide > rect.a.lengthSquared() || tempDirDotSide < 0.0) {
                // Didn't hit
                return {};
            }

            tempDirDotSide = tempDir * rect.b;
            if (tempDirDotSide > rect.b.lengthSquared() || tempDirDotSide < 0.0) {
                // Didn't hit
                return {};
            }

            return {static_cast<float>(t)};
        }
    } // namespace OnRayCastAspect
    #endif
} // namespace Smurf