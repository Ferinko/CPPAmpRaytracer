#pragma once

#include <chrono>
#include <ctime>
#include <iostream>
#include <utility>
#include <string>

namespace Smurf {
    class Timer {
        std::chrono::time_point<std::chrono::system_clock> startTime, endTime;
    public:
        std::chrono::time_point<std::chrono::system_clock> start() {
            startTime = std::chrono::system_clock::now();
            return startTime;
        }
        std::chrono::time_point<std::chrono::system_clock> end() {
            endTime = std::chrono::system_clock::now();
            return endTime;
        }
        std::string elapsed() const {
            std::chrono::duration<double> elapsed_seconds = endTime - startTime;
            return std::string("Elapsed time: ") + std::to_string(elapsed_seconds.count()) + "s";
        }
        template <typename F, typename... Args>
        auto timedOperation(F closure, Args&&... args) -> std::pair<decltype(closure(std::forward<Args>(args)...)), std::string> {
            start();
            auto result = closure(std::forward<Args>(args)...);
            end();
            return std::make_pair(result, elapsed());
        }
    };

} // namespace Smurf