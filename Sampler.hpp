#pragma once

#include "Vec2.hpp"
#include "Wheels.hpp"
#include "Settings.hpp"

#include <array>
#include <algorithm>
#include <numeric>
#include <vector>

namespace Smurf {
    class Sampler {
    public:
        Sampler() : counter{0}, offset{0}, randomEngineLocalRef{Utils::RandomEngine::instance()} {
            // Indices could be const but for the time being, it's too much of a hassle to random_shufle iota into initializer list
            std::array<int, Settings::NumSamples> temp;
            std::iota(std::begin(temp), std::end(temp), 0);
            for (int group = 0; group < Settings::Internal::NumSampleGroups; ++group) {
                std::random_shuffle(std::begin(temp), std::end(temp));
                for (int i = 0; i < Settings::NumSamples; ++i) {
                    indices[group * Settings::NumSamples + i] = temp[i];
                }
            }
            generateJitteredSamples();
            randomEngineLocalRef.setIntegralCustomRange(0, Settings::Internal::NumSampleGroups - 1);
        }

        // Distribute samples over a unit suare
        Vec2<double> sampleAtomicSquare() {
            if (counter % Settings::NumSamples == 0) {
                offset = randomEngineLocalRef.randIntCustom() * Settings::NumSamples;
            }
            return samples[offset + indices[offset + counter++ % Settings::NumSamples]];
        }

        // Shirley's square to disc mapping
        void sampleAtomicCircle() {
            Vec2<double> samplePoint;
            double radius;
            double angle;
            int idx = 0;
            for (auto&& sample : samples) {
                samplePoint.x = 2.0 * sample.x - 1.0;
                samplePoint.y = 2.0 * sample.y - 1.0;
                if (samplePoint.x > -samplePoint.y) {
                    if (samplePoint.x > samplePoint.y) {
                        radius = samplePoint.x;
                        angle = samplePoint.y / samplePoint.x;
                    } else {
                        radius = samplePoint.y;
                        angle = 2 - samplePoint.x / samplePoint.y;
                    }
                } else {
                    if (samplePoint.x < samplePoint.y) {
                        radius = -samplePoint.x;
                        angle = 4 + samplePoint.y / samplePoint.x;
                    } else {
                        radius = -samplePoint.y;
                        if (samplePoint.y == 0) {
                            angle = 0;
                        } else {
                            angle = 6 - samplePoint.x / samplePoint.y;
                        }
                    }
                }
                angle *= PiQuarter;
                shirleyDiscSamples[idx].x = cos(angle) * radius;
                shirleyDiscSamples[idx].x = sin(angle) * radius;
                ++idx;
            }
        }

        // Square to hemisphere mapping
        void sampleAtomicHemisphere() {
            int idx = 0;
            const auto e = 1.0 / (1.0 + Settings::Internal::HemisphereMapFactor);
            for (auto&& sample : samples) {
                auto twoPiPhi = TwoPi * sample.x;
                double cosAngle = cos(twoPiPhi);
                double sinAngle = sin(twoPiPhi);
                double cosPhi = pow((1.0 - samples[idx].y), e);
                double sinPhi = sqrt(1.0 - cosPhi * cosPhi);
                hemisphereSamples[idx] = Vec3<double>(sinPhi * cosAngle, sinPhi * sinAngle, cosPhi);
                ++idx;
            }
        }

        const std::array<Vec2<double>, Settings::NumSamples * Settings::Internal::NumSampleGroups>& getSamples() const {
            return samples;
        }

        const std::array<int, Settings::NumSamples * Settings::Internal::NumSampleGroups>& getIndices() const {
            return indices;
        }
    private:
        // Jittered sampler
         void generateJitteredSamples() {
            for (int group = 0; group < Settings::Internal::NumSampleGroups; ++group) {
                for (int pRow = 0; pRow < CompileTime::SimpleSquareRoot<Settings::NumSamples>::value; ++pRow) {
                    for (int pCol = 0; pCol < CompileTime::SimpleSquareRoot<Settings::NumSamples>::value; ++pCol) {
                        samples[group * Settings::NumSamples
                               + pRow * CompileTime::SimpleSquareRoot<Settings::NumSamples>::value
                               + pCol] = {
                                   (pCol + randomEngineLocalRef.randReal()) / CompileTime::SimpleSquareRoot<Settings::NumSamples>::value,
                                   (pRow + randomEngineLocalRef.randReal()) / CompileTime::SimpleSquareRoot<Settings::NumSamples>::value
                               };
                    }
                }
            }
        }
    private:
        std::array<int, Settings::NumSamples * Settings::Internal::NumSampleGroups> indices;
        std::array<Vec2<double>, Settings::NumSamples * Settings::Internal::NumSampleGroups> samples;
        std::array<Vec2<double>, Settings::NumSamples * Settings::Internal::NumSampleGroups> shirleyDiscSamples;
        std::array<Vec3<double>, Settings::NumSamples * Settings::Internal::NumSampleGroups> hemisphereSamples;
        int counter;
        int offset;
        Utils::RandomEngine& randomEngineLocalRef;
    };
} // namespace Smurf