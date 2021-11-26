#pragma once

#include <amp.h>

#include <ostream>

namespace Smurf {
    void printPCInfo(std::wostream& os) {
        auto accelerators = Concurrency::accelerator::get_all();
        for (auto && elem : accelerators) {
            os << elem.description << std::endl;
        }
    }

    void printRayTraceInfo(std::wostream& os) {
        os << L"Resolution: " << Settings::HRes << L" * " << Settings::VRes << "\n"
           << L"Antialiasing: " << Settings::NumSamples << "\n"
           << L"Shading: " << L"Simple lights" << "\n"
           << L"Materials: " << L"Matte" << "\n"
           << std::endl; 
    }
} // namespace Smurf