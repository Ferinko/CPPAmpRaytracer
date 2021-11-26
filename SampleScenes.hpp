#pragma once

#include "Scene.hpp"
#include "Wheels.hpp"
#include "GeometricObject.hpp"
#include "Material.hpp"
#include "Light.hpp"

#include <memory>

using Smurf::Utils::make_unique;

namespace Smurf {
    namespace Scenes {
        // TODO - rework so that the repeated parts are factored out
        std::unique_ptr<Scene> constructQuasiCube() {
            Camera camera{{110.0, 105.0, 450.0},
            {0.0, 0.0, 0.0},
            {0, 1, 0},
            1000.0};
            auto scene = make_unique<Scene>(camera, Color{0.0, 0.0, 0.0});

            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ -25, 25, 0 + 10 }, Vec3<double>{ 50, 0, 0 }, Vec3<double>{ 0, -50, 0 }, Vec3<double>{ 0, 0, 1 })); // Front
            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ -25 - 10, 25, 0 }, Vec3<double>{ 0, 0, -50 }, Vec3<double>{ 0, -50, 0 }, Vec3<double>{ 1, 0, 0 })); // Left
            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ -25, 25 + 10, 0 }, Vec3<double>{ 0, 0, -50 }, Vec3<double>{ 50, 0, 0 }, Vec3<double>{ 0, -1, 0 })); // Top

            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ 25, -25, -50 - 10 }, Vec3<double>{ -50, 0, 0 }, Vec3<double>{ 0, 50, 0 }, Vec3<double>{ 0, 0, -1 })); // Back
            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ 25 + 10, -25, -50 }, Vec3<double>{ 0, 50, 0 }, Vec3<double>{ 0, 0, 50 }, Vec3<double>{ -1, 0, 0 })); // Right
            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ 25, -25 - 10, -50 }, Vec3<double>{ -50, 0, 0 }, Vec3<double>{ 0, 0, 50 }, Vec3<double>{ 0, 1, 0 })); // Bottom

            scene->addToScene(make_unique<Sphere>(Vec3<double>(177, 0, -150), 50));
            scene->addToScene(make_unique<Plane>(Vec3<double>(0, -10, 0), Vec3<double>(0, 1, 0), Color(1.0, 1.0, 0.0)));

            return scene;
        }

        std::unique_ptr<Scene> constructSampleSpheres() {
            Camera camera{{120.0, 75.0, 450.0},
            {0.0, 0.0, 0.0},
            {0, 1, 0},
            400};
            auto scene = make_unique<Scene>(camera, Color{0.0, 0.0, 0.0});

            scene->addToScene(make_unique<Sphere>(Vec3<double>(0, 0, 0), 100));
            scene->addToScene(make_unique<Sphere>(Vec3<double>(177, 0, -150), 50));
            scene->addToScene(make_unique<Plane>(Vec3<double>(0, -10, 0), Vec3<double>(0, 1, 0), Color(1.0, 1.0, 0.0)));
            
            return scene;
        }

        std::unique_ptr<Scene> constructSceneGPU2() {
            Camera camera{ { 100.0, 0.0, 300.0 },
                { 0.0, 0.0, 0.0 },
                { 0, 1, 0 },
                1000.0
            };

            auto scene = make_unique<Scene>(camera, Color { 0.0, 0.0, 0.0 });

            Matte orangeMaterial;
            orangeMaterial.setAmbientIntensity(0.0F);
            orangeMaterial.setDiffuseIntensity(1.0F);
            orangeMaterial.setColor({ 0.65F, 0.35F, 0.1F });

            Matte invisible;
            invisible.setAmbientIntensity(0.0F);
            invisible.setDiffuseIntensity(1.0F);
            invisible.setColor({ 0.5F, 0.2F, 0.0F });




            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ -25, 25, 0 }, Vec3<double>{ 50, 0, 0 }, Vec3<double>{ 0, -50, 0 }, Vec3<double>{ 0, 0, 1 }, invisible)); // Front
            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ -25, 25, 0 }, Vec3<double>{ 0, 0, -50 }, Vec3<double>{ 0, -50, 0 }, Vec3<double>{ 1, 0, 0 }, invisible)); // Left
            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ -25, 25, 0 }, Vec3<double>{ 0, 0, -50 }, Vec3<double>{ 50, 0, 0 }, Vec3<double>{ 0, -1, 0 }, invisible)); // Top

            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ 25, -25, -50}, Vec3<double>{ -50, 0, 0 }, Vec3<double>{ 0, 50, 0 }, Vec3<double>{ 0, 0, -1 }, invisible)); // Back
            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ 25, -25, -50 }, Vec3<double>{ 0, 50, 0 }, Vec3<double>{ 0, 0, 50 }, Vec3<double>{ -1, 0, 0 }, invisible)); // Right
            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ 25, -25, -50 }, Vec3<double>{ -50, 0, 0 }, Vec3<double>{ 0, 0, 50 }, Vec3<double>{ 0, 1, 0 }, invisible)); // Bottom

            Matte greenMaterial;
            greenMaterial.setAmbientIntensity(0.0F);
            greenMaterial.setDiffuseIntensity(1.0F);
            greenMaterial.setColor({ 0.3F, 0.9F, 0.3F });

            Matte blueMaterial;
            blueMaterial.setAmbientIntensity(0.1F);
            blueMaterial.setDiffuseIntensity(1.0F);
            blueMaterial.setColor({ 0.0F, 0.0F, 0.75F });

            Matte yellow;
            yellow.setAmbientIntensity(0.1F);
            yellow.setDiffuseIntensity(1.0F);
            yellow.setColor({ 1.0F, 1.0F, 1.0F });

            scene->addToScene(make_unique<Sphere>(Vec3<double>(100000, 0, 0), 50, blueMaterial));
            //scene->addToScene(make_unique<Sphere>(Vec3<double>(-300, 105, 0), 100, greenMaterial));
            //scene->addToScene(make_unique<Plane>(Vec3<double>(0, -10, 0), Vec3<double>(0, 1, 0), Color(1.0, 1.0, 0.0), greenMaterial));
            scene->addToScene(make_unique<Plane>(Vec3<double>(-100000, 0, 0), Vec3<double>(1, 0, 0), Color(1.0, 1.0, 0.0), yellow));

            //scene->addToScene(make_unique<Sphere>(Vec3<double>(-250, 0, 0), 4, blueMaterial));
            //scene->addLight(PointLight{ { 1.0F, 0.0F, 0.0F }, 1.0F, { -250, 0, 0 } });
            scene->addLight(PointLight{ { 1.0F, 1.0F, 1.0F }, 1.0F, { -250, 250, 250 } });
            /*scene->addLight(PointLight{ { 0.0F, 1.0F, 0.0F }, 1.0F, { -211, 240, 40 } });
            scene->addLight(PointLight{ { 0.0F, 0.0F, 1.0F }, 1.0F, { -250, 0, -250 } });*/
            /*scene->addLight(PointLight{ { 0.0F, 1.0F, 0.0F }, 0.0F, { 0, 100, 0 } });
            scene->addLight(PointLight{ { 0.0F, 0.0F, 1.0F }, 0.0F, { 50, 100, 100 } });*/

            scene->addLight(DirectionalLight{ Color{ 1.0F, 1.0F, 1.0F }, 0.0F, { -0.65, 0.35, 0 } });
            scene->ambientLight = AmbientLight{};

            return scene;
        }

        std::unique_ptr<Scene> constructSceneGPU0() {
            Camera camera{ {0.0, 200.0, 1000.0},
            { 0.0, 0.0, 0.0 },
            { 0, 1, 0 },
            1000.0 };
            auto scene = make_unique<Scene>(camera, Color{ 0.0F, 0.0F, 0.0F });

            Matte orangeMaterial;
            orangeMaterial.setAmbientIntensity(0.0F);
            orangeMaterial.setDiffuseIntensity(1.0F);
            orangeMaterial.setColor({ 0.65F, 0.35F, 0.1F });

            Matte invisible;
            invisible.setAmbientIntensity(0.0F);
            invisible.setDiffuseIntensity(1.0F);
            invisible.setColor({ 0.5F, 0.2F, 0.0F });

            scene->addToScene(make_unique<Rectangle>(Vec3<double>{ -5000, 0, 0 }, Vec3<double>{ 0.1, 0, 0 }, Vec3<double>{ 0, 0.1, 0 }, Vec3<double>{ 0, 0, 1 }, invisible)); // Front

            Matte greenMaterial;
            greenMaterial.setAmbientIntensity(0.6F);
            greenMaterial.setDiffuseIntensity(1.0F);
            greenMaterial.setColor({ 0.3F, 0.9F, 0.3F });

            Glossy greenMatGlossy;
            greenMatGlossy.setAmbientIntensity(0.6F);
            greenMatGlossy.setDiffuseIntensity(1.0F);
            greenMatGlossy.setColor({ 0.3F, 0.9F, 0.3F });
            greenMatGlossy.setSpecularExponent(40.0F);
            greenMatGlossy.setSpecularIntensity(0.7F);

            Glossy blueMaterial;
            blueMaterial.setAmbientIntensity(0.5F);
            blueMaterial.setDiffuseIntensity(0.7F);
            blueMaterial.setColor({ 0.0F, 0.75F, 0.75F });
            blueMaterial.setSpecularExponent(3.0F);
            blueMaterial.setSpecularIntensity(0.7F);

            Matte yellow;
            yellow.setAmbientIntensity(0.5F);
            yellow.setDiffuseIntensity(1.0F);
            yellow.setColor({ 1.0F, 1.0F, 1.0F });

            Glossy nMaterial;
            nMaterial.setAmbientIntensity(0.5F);
            nMaterial.setDiffuseIntensity(0.7F);
            nMaterial.setColor({ 0.75F, 0.0F, 0.0F });
            nMaterial.setSpecularExponent(3.0F);
            nMaterial.setSpecularIntensity(0.7F);

            Glossy mMaterial;
            mMaterial.setAmbientIntensity(0.5F);
            mMaterial.setDiffuseIntensity(0.7F);
            mMaterial.setColor({ 0.1F, 0.80F, 0.2F });
            mMaterial.setSpecularExponent(15.0F);
            mMaterial.setSpecularIntensity(1.0F);

            scene->addToScene(make_unique<Sphere>(Vec3<double>(-200, 100, -50), 110, blueMaterial));
            scene->addToScene(make_unique<Sphere>(Vec3<double>(0, 100, 0), 110, yellow));
            scene->addToScene(make_unique<Sphere>(Vec3<double>(200, 100, 50), 110, mMaterial));
            scene->addToScene(make_unique<Sphere>(Vec3<double>(200, 100, 250), 110, nMaterial));
            //scene->addToScene(make_unique<Plane>(Vec3<double>(0, -10, 0), Vec3<double>(0, 1, 0), Color(1.0, 1.0, 0.0), greenMatGlossy));
            scene->addToScene(make_unique<Plane>(Vec3<double>(0, -10, 0), Vec3<double>(0, 1, 0), Color(1.0, 1.0, 0.0), greenMatGlossy));

            scene->addLight(PointLight{ { 1.0F, 1.0F, 1.0F }, 1.0F, { -250, 250, 250 } });
            scene->addLight(PointLight{ { 1.0F, 1.0F, 1.0F }, 1.0F, { 250, 150, -250 } });

            scene->addLight(DirectionalLight{ Color{ 1.0F, 1.0F, 1.0F }, 1.0F, { -0.65, 0.35, 0 } });
            scene->ambientLight = AmbientLight{};

            return scene;
        }

    } // namespace Scenes
} // namespace Smurf