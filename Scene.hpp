#pragma once

#include "GeometricObject.hpp"
#include "Material.hpp"
#include "Color.hpp"
#include "Ray.hpp"
#include "Pixel.hpp"
#include "Bitmap.hpp"
#include "Timer.hpp"
#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Settings.hpp"
#include "Sampler.hpp"
#include "Camera.hpp"
#include "BRDF.hpp"
#include "Light.hpp"

#ifdef USE_AMP
#include <amp.h>
#include <amp_math.h>
#undef max
#undef min
#endif

#include <vector>
#include <memory>
#include <fstream>
#include <iostream>

namespace Smurf {
    static const double PixelSize = 1.0;
    static const double HalfPixelSize = 0.5;

    class Scene {
        Camera camera;
        std::vector<std::unique_ptr<GeometricObject>> objects;
        Color background;
        std::unique_ptr<Sampler> sampler;
        std::vector<PointLight> pointLights; // Void of inheritance for the time being, AMP's fault
        std::vector<DirectionalLight> directionalLights;
    public:
        AmbientLight ambientLight;

        Scene() : background{Color(0.0F, 0.0F, 0.0F)}, sampler{Utils::make_unique<Sampler>()} { }
        Scene(Camera camera, Color bgColor) : camera{camera}, background{bgColor}, sampler{Utils::make_unique<Sampler>()} { }
        
        std::vector<Pixel> rayTraceScene() {
            std::vector<Pixel> result;
            Ray ray;
            ray.origin = camera.getEye();
            Color resultColor = background;
            Vec2<double> samplePoint;
            Vec2<double> pixel;

            std::cout << "Settings: \n" <<
                         "Resolution: " << Settings::HRes << " * " << Settings::VRes << "\n" <<
                         "Number of samples: " << Settings::NumSamples << std::endl;

            std::cout << "Raytracing (CPU)." << std::endl;
            Timer timer;
            timer.start();

            for (int row = 0; row < Settings::VRes; ++row) {
                for (int col = 0; col < Settings::HRes; ++col) {
                    for (int sample = 0; sample < Settings::NumSamples; ++sample) {
                        samplePoint = sampler->sampleAtomicSquare();
                        pixel.x = col - HalfPixelSize * Settings::HRes + samplePoint.x;
                        pixel.y = row - HalfPixelSize * Settings::VRes + samplePoint.y;
                        ray.direction = camera.inferRayDirection(pixel);
                        auto hit = hitAllObjects(ray);
                        resultColor += hit ? hit->second : background;
                    }
                    resultColor /= Settings::NumSamples;
                    result.emplace_back(resultColor);
                    resultColor = background;
                }
            }

            timer.end();

            std::cout << "Raytracing finished.\n" << "Time elapsed: " << timer.elapsed() << std::endl;  
            return result;
        }

        std::vector<Pixel> rayTraceSceneGPU() {
            // AMP-specific initialization

            // Ready-up the sampler and prepare indices
            const auto& samples = sampler->getSamples();
            const auto& indices = sampler->getIndices();

            std::vector<int> offsets;
            offsets.reserve(Settings::HRes * Settings::VRes);

            // Randomize sample-groups used before launching the kernel
            auto& randEngineRef = Utils::RandomEngine::instance();
            randEngineRef.setIntegralCustomRange(0, Settings::Internal::NumSampleGroups - 1);
            for (int i = 0; i < Settings::HRes * Settings::VRes; ++i) {
                offsets.push_back(randEngineRef.randIntCustom());
            }

            // Initialize pixels to black
            std::vector<Color> initialScene(Settings::HRes * Settings::VRes);
            
            // De-virtualize objects
            // TODO - Replace with emplace_back(std::move()) -> add proper rvalue ctors
            std::vector<g_Sphere> spheres;
            std::vector<g_Plane> planes;
            std::vector<g_Rectangle> rectangles;
            for (auto&& object: objects) {
                if (dynamic_cast<Sphere*>(object.get())) {
                    auto center = dynamic_cast<Sphere*>(object.get())->getCenter();
                    auto radius = dynamic_cast<Sphere*>(object.get())->getRadius();
                    auto temp = object.get();
                    Matte matte;
                    Glossy glossy;
                    switch (temp->active) {
                        case ActiveMaterial::ActiveMatte:
                            matte = dynamic_cast<Sphere*>(temp)->matte;
                            spheres.push_back(g_Sphere(center, radius, matte));
                            break;
                        case ActiveMaterial::ActiveGlossy:
                            glossy = dynamic_cast<Sphere*>(temp)->glossy;
                            spheres.push_back(g_Sphere(center, radius, glossy));
                            break;
                    } 
                } else if (dynamic_cast<Plane*>(object.get())) {
                    auto point = dynamic_cast<Plane*>(object.get())->getPoint();
                    auto normal = dynamic_cast<Plane*>(object.get())->getNormal();
                    auto temp = object.get();
                    Matte matte;
                    Glossy glossy;
                    switch (temp->active) {
                    case ActiveMaterial::ActiveMatte:
                        matte = dynamic_cast<Plane*>(temp)->matte;
                        planes.push_back(g_Plane(point, normal, matte));
                        break;
                    case ActiveMaterial::ActiveGlossy:
                        glossy = dynamic_cast<Plane*>(temp)->glossy;
                        planes.push_back(g_Plane(point, normal, glossy));
                        break;
                    }
                } else {
                    auto point = dynamic_cast<Rectangle*>(object.get())->getPoint();
                    auto a = dynamic_cast<Rectangle*>(object.get())->getA();
                    auto b = dynamic_cast<Rectangle*>(object.get())->getB();
                    auto normal = dynamic_cast<Rectangle*>(object.get())->getNormal();
                    auto temp = object.get();
                    Matte matte;
                    Glossy glossy;
                    switch (temp->active) {
                    case ActiveMaterial::ActiveMatte:
                        matte = dynamic_cast<Rectangle*>(temp)->matte;
                        rectangles.emplace_back(std::move(g_Rectangle(point, a, b, normal, matte)));
                        break;
                    case ActiveMaterial::ActiveGlossy:
                        glossy = dynamic_cast<Rectangle*>(temp)->glossy;
                        rectangles.emplace_back(std::move(g_Rectangle(point, a, b, normal, glossy)));
                        break;
                    }
                }
            }

            const int numSpheres = spheres.size();
            const int numPlanes = planes.size();
            const int numRects = rectangles.size();
            const int numDirLights = directionalLights.size();
            const int numPointLights = pointLights.size();
            
            // Copy to GPU
            const Concurrency::array<int, 1> g_Offsets{Settings::HRes * Settings::VRes, std::begin(offsets), std::end(offsets)};
            const Concurrency::array<int, 1> g_Indices{ Settings::NumSamples * Settings::Internal::NumSampleGroups, indices.data() };
            const Concurrency::array<Vec2<double>, 1> g_Samples{Settings::NumSamples * Settings::Internal::NumSampleGroups, samples.data()};
            const Concurrency::array<g_Sphere, 1> g_Spheres{numSpheres, std::begin(spheres), std::end(spheres)};
            const Concurrency::array<g_Plane, 1> g_Planes{numPlanes, std::begin(planes), std::end(planes)};
            const Concurrency::array<g_Rectangle, 1> g_Rectangles{numRects, std::begin(rectangles), std::end(rectangles)};
            const Concurrency::array<DirectionalLight, 1> g_DirectionalLights{numDirLights, std::begin(directionalLights), std::end(directionalLights)};
            const Concurrency::array<PointLight, 1> g_PointLights{numPointLights, std::begin(pointLights), std::end(pointLights)};
            Concurrency::array_view<Color, 1> g_Result{Settings::HRes * Settings::VRes, initialScene};

            // Pull out the camera settings
            const auto camera = this->camera;

            // Pull out background and ambient light
            const auto bg = background;
            auto ambientLight = this->ambientLight;

            // Compute reciprocal of NumSamples
            const float oneOverNumSamples = 1.0F / Settings::NumSamples;

            std::cout << "Raytracing (GPU)." << std::endl;
            Timer timer;
            timer.start();

            // Raytrace
            Concurrency::parallel_for_each(g_Result.extent, [=, &g_Samples,
                                                                &g_Offsets,
                                                                &g_Indices,
                                                                &g_Spheres,
                                                                &g_Planes,
                                                                &g_Rectangles,
                                                                &g_DirectionalLights,
                                                                &g_PointLights]
            (Concurrency::index<1> idx) restrict(amp) {
                Ray ray;
                ray.origin = camera.getEye();
                Color resultColor{bg};
                Vec2<double> pixel;
                for (int sample = 0; sample < Settings::NumSamples; ++sample) {
                    auto samplePoint = g_Samples[g_Offsets[idx] * Settings::NumSamples + g_Indices[sample + g_Offsets[idx] * Settings::NumSamples]];
                    pixel.x = (idx[0] % Settings::HRes) - 0.5 * Settings::HRes + samplePoint.x;                    
                    pixel.y = (idx[0] / Settings::HRes) - 0.5 * Settings::VRes + samplePoint.y;
                    ray.direction = camera.inferRayDirection(pixel);
                    auto hit = g_hitAllObjects(ray, g_Spheres, g_Planes, g_Rectangles, numSpheres, numPlanes, numRects);
                    resultColor += hit.hasHit ? dispatchMaterial(hit, ray,
                                                                 ambientLight, g_Spheres, g_Planes, g_Rectangles, g_DirectionalLights,
                                                                 g_PointLights, numSpheres, numPlanes, numRects, numDirLights, numPointLights)
                                              : bg;
                }
                resultColor *= oneOverNumSamples;
                g_Result[idx] = resultColor;
            });

        timer.end();
        
        g_Result.synchronize();
        std::cout << "Raytracing finished.\n" << "Time elapsed: " << timer.elapsed() << std::endl;
        std::vector<Pixel> finalScene(std::begin(initialScene), std::end(initialScene));
        return finalScene;
        }

        void renderScene(const std::vector<Pixel>& scene) const {
            std::ofstream bitmap;
            bitmap.open(Utils::getTimestamp() + ".bmp", std::ios::binary);
            bitmap << FileFormat::Bitmap(Settings::HRes, Settings::VRes, scene);
            bitmap.close();
        }

        boost::optional<std::pair<RayHit, Color>> hitAllObjects(const Ray& ray) const {
            double closestObjectT = std::numeric_limits<double>::max();
            int closestObjectIndex; // Cannot use to tell whether something's been hit as infinity as an option
            bool hasHit = false;

            for (unsigned objectIdx = 0; objectIdx < objects.size(); ++objectIdx) {
                auto hit = objects[objectIdx]->onRayCast(ray);
                if (hit && hit->tMin < closestObjectT) {
                    closestObjectT = hit->tMin;
                    closestObjectIndex = objectIdx;
                    hasHit = true;
                }
            }

            if (!hasHit) return {};

            return {
                std::make_pair(RayHit(closestObjectT), Color(objects[closestObjectIndex]->color))
            };
        }

        friend g_RayHit g_hitAllObjects(const Ray ray, const Concurrency::array<g_Sphere>& spheres,
                                        const Concurrency::array<g_Plane>& planes,
                                        const Concurrency::array<g_Rectangle>& rectangles,
                                        int numSpheres,
                                        int numPlanes,
                                        int numRects) restrict(amp) {       
            g_RayHit result;
            result.tMin = 1.79769e+308;
            result.hasHit = false;
            enum class PrimitiveHit { Sphere, Plane, Rectangle } lastHitType;
            unsigned lastHitIdx;

            // horrible - difficult to work with Concurrency::arrays, begin and end can't really be taken
            #define REGISTER_PRIMITIVE(container, primitiveFullEnumName, size) \
                for (int i = 0; i < size; ++i) { \
                    auto hit = OnRayCastAspect::onRayCast(container[i], ray); \
                        if (hit && hit.tMin < result.tMin) { \
                            result.tMin = hit.tMin; \
                            lastHitType = primitiveFullEnumName; \
                            lastHitIdx = i; \
                            result.hasHit = true; \
                            result.active = hit.active; \
                            result.hitPoint = ray.origin + hit.tMin * ray.direction; \
                            result.normal = hit.normal; \
                        } \
                    }

            REGISTER_PRIMITIVE(spheres, PrimitiveHit::Sphere, numSpheres);
            REGISTER_PRIMITIVE(planes, PrimitiveHit::Plane, numPlanes);
            REGISTER_PRIMITIVE(rectangles, PrimitiveHit::Rectangle, numRects);

            #undef REGISTER_PRIMITIVE

            if (!result.hasHit) return result;

            switch (lastHitType) {
                case PrimitiveHit::Sphere:
                    switch (result.active) {
                        case ActiveMaterial::ActiveMatte:
                            result.matte = spheres[lastHitIdx].matte;
                            break;
                        case ActiveMaterial::ActiveGlossy:
                            result.glossy = spheres[lastHitIdx].glossy;
                            break;
                    }
                    break;
                case PrimitiveHit::Plane:
                    switch (result.active) {
                        case ActiveMaterial::ActiveMatte:
                            result.matte = planes[lastHitIdx].matte;
                            break;
                        case ActiveMaterial::ActiveGlossy:
                            result.glossy = planes[lastHitIdx].glossy;
                            break;
                    }
                    break;
                case PrimitiveHit::Rectangle:
                    switch (result.active) {
                        case ActiveMaterial::ActiveMatte:
                            result.matte = rectangles[lastHitIdx].matte;
                            break;
                        case ActiveMaterial::ActiveGlossy:
                            result.glossy = rectangles[lastHitIdx].glossy;
                            break;
                    }
                    break;
            }

            return result;
        }

        friend Color shade(const Matte material,
                           const Ray ray,
                           const Vec3<double> normal,
                           const Vec3<double> hitPoint,
                           const AmbientLight ambientLight,
                           const Concurrency::array<g_Sphere>& spheres,
                           const Concurrency::array<g_Plane>& planes,
                           const Concurrency::array<g_Rectangle>& rectangles,
                           const Concurrency::array<DirectionalLight>& directionalLights,
                           const Concurrency::array<PointLight>& pointLights,
                           const int numSpheres,
                           const int numPlanes,
                           const int numRects,
                           const int numDirectionalLights,
                           const int numPointLights) restrict(amp) {
            auto flippedDirection = -ray.direction;
            auto result = material.getBrdfAmbient().rho() * ambientLight.getRadiance();

            for (int dirLight = 0; dirLight < numDirectionalLights; ++dirLight) {
                auto direction = directionalLights[dirLight].getDirection();
                auto normalDotDirection = normal * direction;
                if (normalDotDirection > 0.0) {
                    Ray shadowRay(hitPoint, direction);
                    if (inShadow(spheres, planes, rectangles, shadowRay, directionalLights[dirLight], numSpheres, numPlanes, numRects)) {
                        continue;
                    }
                    result += material.getBrdfDiffuse().diffuseF() * directionalLights[dirLight].getRadiance() * static_cast<float>(normalDotDirection);
                }
            }

            for (int pointLight = 0; pointLight < numPointLights; ++pointLight) {
                auto direction = pointLights[pointLight].getDirection(hitPoint);
                auto normalDotDirection = normal * direction;
                    Ray shadowRay(hitPoint, direction);
                    if (inShadow(spheres, planes, rectangles, shadowRay, pointLights[pointLight], numSpheres, numPlanes, numRects)) {
                        continue;
                    }
                if (normalDotDirection > 0.0) {
                    result += material.getBrdfDiffuse().diffuseF() * pointLights[pointLight].getRadiance() * static_cast<float>(normalDotDirection);
                }
            }

            return result;
        }

        friend Color shade(const Glossy glossy,
                           const Ray ray,
                           const Vec3<double> normal,
                           const Vec3<double> hitPoint,
                           const AmbientLight ambientLight,
                           const Concurrency::array<g_Sphere>& spheres,
                           const Concurrency::array<g_Plane>& planes,
                           const Concurrency::array<g_Rectangle>& rectangles,
                           const Concurrency::array<DirectionalLight>& directionalLights,
                           const Concurrency::array<PointLight>& pointLights,
                           const int numSpheres,
                           const int numPlanes,
                           const int numRects,
                           const int numDirectionalLights,
                           const int numPointLights) restrict(amp) {
            auto flippedDirection = -ray.direction;
            auto result = glossy.getBrdfAmbient().rho() * ambientLight.getRadiance();

            for (int dirLight = 0; dirLight < numDirectionalLights; ++dirLight) {
                const auto& direction = directionalLights[dirLight].getDirection();
                auto normalDotDirection = normal * direction;
                if (normalDotDirection > 0.0) {
                    Ray shadowRay(hitPoint, direction);
                    if (inShadow(spheres, planes, rectangles, shadowRay, directionalLights[dirLight], numSpheres, numPlanes, numRects)) {
                        continue;
                    }
                    result += (glossy.getBrdfSpecular().diffuseF(normal, flippedDirection, direction) + glossy.getBrdfDiffuse().diffuseF())
                        * directionalLights[dirLight].getRadiance() * static_cast<float>(normalDotDirection);
                }
            }

            for (int pointLight = 0; pointLight < numPointLights; ++pointLight) {
                const auto& direction = pointLights[pointLight].getDirection(hitPoint);
                auto normalDotDirection = normal * direction;
                if (normalDotDirection > 0.0) {
                    Ray shadowRay(hitPoint, direction);
                    if (inShadow(spheres, planes, rectangles, shadowRay, pointLights[pointLight], numSpheres, numPlanes, numRects)) {
                        continue;
                    }
                    result += (glossy.getBrdfSpecular().diffuseF(normal, flippedDirection, direction) + glossy.getBrdfDiffuse().diffuseF())
                        * pointLights[pointLight].getRadiance() * static_cast<float>(normalDotDirection);
                }
            }

            return result;
        }

        friend Color dispatchMaterial(g_RayHit hit, Ray ray,
                                      AmbientLight ambientLight,
                                      const Concurrency::array<g_Sphere>& spheres,
                                      const Concurrency::array<g_Plane>& planes,
                                      const Concurrency::array<g_Rectangle>& rectangles,
                                      const Concurrency::array<DirectionalLight>& g_DirectionalLights,
                                      const Concurrency::array<PointLight>& g_PointLights,
                                      int numSpheres, int numPlanes, int numRects,
                                      int numDirLights, int numPointLights) restrict(amp) {
            switch (hit.active) {
            case ActiveMaterial::ActiveMatte:
                return shade(hit.matte, ray, hit.normal, hit.hitPoint,
                             ambientLight, spheres, planes, rectangles, g_DirectionalLights, g_PointLights,
                             numSpheres, numPlanes, numRects, numDirLights, numPointLights);
                break;
            case ActiveMaterial::ActiveGlossy:
                return shade(hit.glossy, ray, hit.normal, hit.hitPoint,
                             ambientLight, spheres, planes, rectangles, g_DirectionalLights, g_PointLights,
                             numSpheres, numPlanes, numRects, numDirLights, numPointLights);
                break;
            default:
                return {};
            }
        }

        friend bool inShadow(const Concurrency::array<g_Sphere>& spheres,
                             const Concurrency::array<g_Plane>& planes,
                             const Concurrency::array<g_Rectangle>& rectangles,
                             Ray ray,
                             PointLight light,
                             const int numSpheres, const int numPlanes, const int numRectangles) restrict(amp) {
            auto dist = light.location.distance(ray.origin);

            #define REGISTER_SHADOW_PRIMITIVE(container, size) \
                for (int i = 0; i < size; ++i) { \
                    auto hit = OnRayCastAspect::onShadowRayCast(container[i], ray); \
                    if (hit && hit.t < dist) { \
                        return true; \
                    } \
                }
            REGISTER_SHADOW_PRIMITIVE(spheres, numSpheres);
            REGISTER_SHADOW_PRIMITIVE(planes, numPlanes);
            REGISTER_SHADOW_PRIMITIVE(rectangles, numRectangles);

            #undef REGISTER_SHADOW_PRIMITIVE
            return false;
        }

        friend bool inShadow(const Concurrency::array<g_Sphere>& spheres,
                             const Concurrency::array<g_Plane>& planes,
                             const Concurrency::array<g_Rectangle>& rectangles,
                             Ray ray,
                             DirectionalLight light,
                             const int numSpheres, const int numPlanes, const int numRectangles) restrict(amp) {
            // Horrible constant is horrible, but works -> just scale the ray far in the direction and shoot for a shadowhit
            auto location = ray.origin - 10000 * light.getDirection();
            auto dist = location.distance(ray.origin);
            #define REGISTER_SHADOW_PRIMITIVE_DIR(container, size) \
                for (int i = 0; i < size; ++i) { \
                    auto hit = OnRayCastAspect::onShadowRayCast(container[i], ray); \
                    if (hit && hit.t < dist) { \
                        return true; \
                    } \
                }
            REGISTER_SHADOW_PRIMITIVE_DIR(spheres, numSpheres);
            REGISTER_SHADOW_PRIMITIVE_DIR(planes, numPlanes);
            REGISTER_SHADOW_PRIMITIVE_DIR(rectangles, numRectangles);

            #undef REGISTER_SHADOW_PRIMITIVE_DIR
            return false;
        }

        void addToScene(std::unique_ptr<GeometricObject> object) {
            objects.emplace_back(std::move(object));
        }

        template <typename T>
        void addLight(T&& light) {
            _dispatchAddLight(std::forward<T>(light));
        }

        void _dispatchAddLight(PointLight&& light) {
            pointLights.emplace_back(std::move(light));
        }

        void _dispatchAddLight(DirectionalLight&& light) {
            directionalLights.emplace_back(std::move(light));
        }
    };
}