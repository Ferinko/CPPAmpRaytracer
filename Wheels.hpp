#pragma once

#include <stdint.h>
#include <memory>
#include <limits>
#include <random>
#include <chrono>
#include <ctime>
#include <algorithm>

#ifdef USE_AMP
#undef max
#undef min
#endif

namespace Smurf {
    typedef unsigned char byte;
    typedef uint16_t word;
    typedef uint32_t dword;
    typedef uint64_t qword;
    const double ReciprocalPi = 1 / std::_Pi;
    const double TwoPi = std::_Pi * 2;
    const double PiHalf = std::_Pi / 2;
    const double PiQuarter = std::_Pi / 4;
    namespace CompileTime {
        // Endian manipulation
        inline byte _swapEndian(byte arg) {
            return arg;
        }

        inline word _swapEndian(word arg) {
            return _byteswap_ushort(arg);
        }

        inline dword _swapEndian(dword arg) {
            return _byteswap_ulong(arg);
        }

        inline qword _swapEndian(qword arg) {
            return _byteswap_uint64(arg);
        }

        template <typename T>
        inline T swapEndian(const T& arg) {
            return _swapEndian(arg);
        }

        template <typename T>
        inline T _swapEndianOutputRef(T& arg) {
            arg = swapEndian(arg);
            return arg;
        }

        // Neutral scalar types
        template <int Size>
        struct _DetermineSize { };

        template <>
        struct _DetermineSize<1> {
            typedef byte type;
        };

        template <>
        struct _DetermineSize<2> {
            typedef word type;
        };

        template <>
        struct _DetermineSize<4> {
            typedef dword type;
        };

        template <>
        struct _DetermineSize<8> {
            typedef qword type;
        };

        // Neutral scalar type concatenators
        template <int N, typename ResultType, typename T, T head, T... tail>
        struct _ConcatHelper {
            static const ResultType temp = head; // Typecast + "memzero"
            static const ResultType value = (temp << (8 * sizeof(T) * N)) | _ConcatHelper<N - 1, ResultType, T, tail...>::value;
        };

        template <typename ResultType, typename T, T head, T... tail>
        struct _ConcatHelper<0, ResultType, T, head, tail...> {
            static const ResultType temp = head; // Typecast + "memzero"
            static const ResultType value = (temp << (8 * sizeof(T) * 0)) | head;
        };

        template <typename T, T head, T... tail>
        struct ConcatScalarTypes {
            typedef typename _DetermineSize<(sizeof(head) + sizeof...(tail)) * sizeof(T)>::type result_type;
            static const int ShiftNTimes = sizeof(result_type) / sizeof(T) - 1;
            static const int value = _ConcatHelper<ShiftNTimes, result_type, T, head, tail...>::value;
        };

        // Foreach elem in tuple
        void applyToAll() { }

        template <typename Lambda, typename... Lambdas>
        void applyToAll(Lambda&& closure, Lambdas&&... closures) {
            std::forward<Lambda>(closure)();
            applyToAll(std::forward<Lambdas>(closures)...);
        }

        template <unsigned... Is>
        struct _Sequence {
            typedef _Sequence<Is...> type;
        };

        template <unsigned Max, unsigned... Is>
        struct _MakeSequence : _MakeSequence<Max - 1, Max - 1, Is...> { };

        template <unsigned... Is>
        struct _MakeSequence<0, Is...> : _Sequence<Is...> { };

        template <typename Tuple, typename Functor, unsigned... Is>
        void _foreachElemInTuple(_Sequence<Is...>, Tuple&& t, Functor&& f) {
            applyToAll(
                [&]{ std::forward<Functor>(f)(std::get<Is>(std::forward<Tuple>(t))); }...
            );
        }

        template <typename Tuple, typename Functor>
        void foreachElemInTuple(Tuple&& t, Functor&& f) {
            _foreachElemInTuple(_MakeSequence<std::tuple_size<typename std::decay<Tuple>::type>::value>(), std::forward<Tuple>(t), std::forward<Functor>(f));
        }

        // Endian foreach adaptor
        struct _SwapEndianOutputRefForward {
            template <typename... Args>
            #ifndef COMPILE_WITH_CTP
            auto operator()(Args && ... args) const -> decltype(Smurf::CompileTime::swapEndian(std::forward<Args>(args)...)) {
                return Smurf::CompileTime::_swapEndianOutputRef(std::forward<Args>(args)...);
            }
            #else
            decltype(auto) operator()(Args && ... args) const {
                return Smurf::CompileTime::_swapEndianOutputRef(std::forward<Args>(args)...);
            }
            #endif
        };

        // Tuple size in bytes without padding
        // TODO - constexpr
        template <typename Tuple, int N>
        struct _RelevantTupleSize {
            static const int value = sizeof(std::tuple_element<N, Tuple>::type) + _RelevantTupleSize<Tuple, N - 1>::value;
        };

        template <typename Tuple>
        struct _RelevantTupleSize<Tuple, 0> {
            static const int value = sizeof(std::tuple_element<0, Tuple>::type);
        };

        template <typename Tuple>
        struct RelevantTupleSize {
            static const int value = _RelevantTupleSize<Tuple, std::tuple_size<Tuple>::value - 1>::value;
        };

        // Simple static square root
        template <int N, int C, bool D>
        struct _SquareRootInternalDispatch { };

        template <int N, int C>
        struct _SquareRootInternalDispatch<N, C, true> {
            static const bool hasResult = ((C - 1) * (C - 1)) == N;
            static const int value = hasResult ? C - 1 : -1;
        };

        template <int N, int C>
        struct _SquareRootInternal {    
            static const int square = C * C;
            static const bool stop = square == N ? true : C > SimpleSquareRoot<N>::MaxIters;
            static const int value = _SquareRootInternalDispatch<N, C + 1, stop>::value;
        };

        template <int N, int C>
        struct _SquareRootInternalDispatch<N, C, false> {
            static const int value = _SquareRootInternal<N, C>::value;
        };

        template <int N>
        struct SimpleSquareRoot {
            static const int MaxIters = N / 2;
            static const int value = _SquareRootInternal<N, 0>::value;
            static_assert(value >= 0, "The number is not a perfect square.");
        };
    } // namespace CompileTime

    namespace Utils {
        // TODO - replace with the standard version eventually
        template <typename T, typename... Args>
        std::unique_ptr<T> make_unique(Args&&... args) {
            return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
        }

        // Write to stream and adapters
        template <typename T>
        inline void streamWrite(std::ostream& os, const T& object) {
            os.write(reinterpret_cast<const char*>(&object), sizeof(T));
        }

        // TODO - C++14 - decltype(auto)
        #define RETURNS(X) -> decltype(X) { return (X); }
        struct _WriteToStreamForward {
            std::ostream& os;
            _WriteToStreamForward(std::ostream& os) : os(os) { }
            template <typename... Args>
            auto operator()(Args && ... args) const
                RETURNS(streamWrite(os, std::forward<Args>(args)...))
        };

        // TODO - C++11 not MSVC 2013 without CTP - constexpr
        static const int storedInNBytes(int numBits) {
            static const int rmndr = numBits % 8;
            static const int bytes = numBits / 8;
            return (bytes < 1 ? 1 : bytes) + (rmndr ? 1 : 0);
        }

        // Uniform pseudo-random number generator
        class RandomEngine {
            std::random_device randomDevice;
            std::mt19937 mersenneTwisterEngine;
            std::uniform_real_distribution<> uniformRealDistribution;
            std::uniform_real_distribution<> *customRealDistribution;
            std::uniform_int_distribution<> uniformIntegralDistribution;
            std::uniform_int_distribution<> *customIntegralDistribution;
            RandomEngine() : mersenneTwisterEngine{randomDevice()},
                             customIntegralDistribution(nullptr),
                             customRealDistribution(nullptr) { }
            ~RandomEngine() {
                delete customIntegralDistribution;
                delete customRealDistribution;
            }
        public:
            static RandomEngine& instance() {
                static RandomEngine instance;
                return instance;
            }

            double randReal() {
                return uniformRealDistribution(mersenneTwisterEngine);
            }

            double randRealCustom() {
                return (*customRealDistribution)(mersenneTwisterEngine);
            }

            int randInt() {
                return uniformIntegralDistribution(mersenneTwisterEngine);
            }

            int randIntCustom() {
                return (*customIntegralDistribution)(mersenneTwisterEngine);
            }

            void setIntegralCustomRange(int rangeBegin, int rangeEnd) {
                if (rangeBegin < 0 || rangeEnd < 0) {
                    return;
                }
                customIntegralDistribution = new std::uniform_int_distribution<>(rangeBegin, rangeEnd);
            }

            void setRealCustomRange(int rangeBegin, int rangeEnd) {
                if (rangeBegin < 0 || rangeEnd < 0) {
                    return;
                }
                customRealDistribution = new std::uniform_real_distribution<>(rangeBegin, rangeEnd);
            }

            std::pair<double, double> getRealCustomRange() const {
                return std::make_pair((*customRealDistribution).min(), (*customRealDistribution).max());
            }

            std::pair<int, int> getIntegralCustomRange() const {
                return std::make_pair((*customIntegralDistribution).min(), (*customIntegralDistribution).max());
            }
        };

        std::string getTimestamp() {
            auto timeStamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            #pragma warning(disable : 4996)
            auto temp = std::string(std::ctime(&timeStamp));
            #pragma warning(default : 4996)
            temp.erase(std::remove_if(std::begin(temp), std::end(temp),
                                      [](char c){ return c == '\n'; }
                                     )
                      );
            // TODO - replace should C++ come with something sensible
            for (char& c : temp) {
                if (c == ':' || c == ' ') c = '_';
            }
            return temp;
        }
    } // namespace Utils
} // namespace Smurf