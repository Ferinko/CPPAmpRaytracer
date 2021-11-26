#pragma once

#include "BRDF.hpp"
#include "Color.hpp"
#include "Light.hpp"
#include "Ray.hpp"

#include <amp.h>

namespace Smurf {

    class Matte {
    public:
        Matte() restrict(cpu, amp) {
            setColor({ 1.0F, 1.0F, 1.0F });
            setAmbientIntensity(1.0F);
            setDiffuseIntensity(1.0F);
        }
        Matte(const Matte& other) restrict(cpu, amp) : brdfAmbient{other.brdfAmbient}, brdfDiffuse{other.brdfDiffuse} { }

        void setAmbientIntensity(float ambientIntensity) restrict(cpu, amp) {
            brdfAmbient.intensity = ambientIntensity;
        }

        void setDiffuseIntensity(float diffuseIntensity) restrict(cpu, amp) {
            brdfDiffuse.intensity = diffuseIntensity;
        }

        void setColor(Color color) restrict(cpu, amp) {
            brdfAmbient.color = color;
            brdfDiffuse.color = color;
        }

        const Lambertian& getBrdfAmbient() const restrict(amp) {
            return brdfAmbient;
        }

        const Lambertian& getBrdfDiffuse() const restrict(amp) {
            return brdfDiffuse;
        }
    private:
        Lambertian brdfAmbient;
        Lambertian brdfDiffuse;
    };

    class Glossy {
    public:
        Glossy() restrict(cpu, amp) {
            setColor({1.0F, 1.0F, 1.0F});
            setAmbientIntensity(1.0F);
            setDiffuseIntensity(1.0F);
            setSpecularIntensity(1.0F);
            setSpecularExponent(1.0F);
        }
        Glossy(const Glossy& other) restrict(cpu, amp) : brdfAmbient{other.brdfAmbient}, brdfDiffuse{other.brdfDiffuse}, brdfSpecular{other.brdfSpecular} { }

        void setAmbientIntensity(float ambientIntensity) restrict(cpu, amp) {
            brdfAmbient.intensity = ambientIntensity;
        }

        void setDiffuseIntensity(float diffuseIntensity) restrict(cpu, amp) {
            brdfDiffuse.intensity = diffuseIntensity;
        }

        void setSpecularIntensity(float specularIntensity) restrict(cpu, amp) {
            brdfSpecular.intensity = specularIntensity;
        }

        void setSpecularExponent(float e) restrict(cpu, amp) {
            brdfSpecular.exponent = e;
        }

        void setColor(Color color) restrict(cpu, amp) {
            brdfAmbient.color = color;
            brdfDiffuse.color = color;
            brdfSpecular.color = color;
        }
        
        const Lambertian& getBrdfAmbient() const restrict(amp) {
            return brdfAmbient;
        }
        
        const Lambertian& getBrdfDiffuse() const restrict(amp) {
            return brdfDiffuse;
        }

        const Specular& getBrdfSpecular() const restrict(amp) {
            return brdfSpecular;
        }

    private:
        Lambertian brdfAmbient;
        Lambertian brdfDiffuse;
        Specular brdfSpecular;
    };
} // namespace Smurf