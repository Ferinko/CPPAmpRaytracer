#pragma once

namespace Smurf {
    namespace Settings {
        const int NumSamples = 16;
        const int HRes = 1920;
        const int VRes = 1200;

        namespace Internal {
            const int NumSupportedPrimitives = 3;
            const double HemisphereMapFactor = 1.0;
            const int NumSampleGroups = 79;
        } // namespace Internal
    } // namespace Settings
} // namespace Smurf