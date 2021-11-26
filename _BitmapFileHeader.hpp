#pragma once

#include "Wheels.hpp"
#include "_DIBHeader.hpp"

#include <tuple>

namespace Smurf {
    struct _BitmapFileHeader {
        _BitmapFileHeader() : fileSize{sizeof(_BitmapFileHeader) + sizeof(_DIBHeader)},
                              reserved1{0},
                              reserved2{0},
                              offset{sizeof(_BitmapFileHeader) + sizeof(_DIBHeader)} { }

        #ifdef BOOST_LITTLE_ENDIAN
            static const word type = Smurf::CompileTime::ConcatScalarTypes<char, 'M', 'B'>::value;
        #else
            static const word type = Smurf::CompileTime::ConcatScalarTypes<char, 'B', 'M'>::value;
        #endif
        dword fileSize;
        word reserved1;
        word reserved2;
        dword offset;

        friend std::ostream& operator<<(std::ostream& os, const _BitmapFileHeader& _bitmapFileHeader) {
            std::tuple<const word&, dword&, word&, word&, dword&> fields(std::tie(_bitmapFileHeader.type,
                                                                                  const_cast<dword&>(_bitmapFileHeader.fileSize),
                                                                                  const_cast<word&>(_bitmapFileHeader.reserved1),
                                                                                  const_cast<word&>(_bitmapFileHeader.reserved2),
                                                                                  const_cast<dword&>(_bitmapFileHeader.offset)));

        #ifndef BOOST_LITTLE_ENDIAN
            Smurf::CompileTime::foreachElemInTuple(fields, Utils::_SwapEndianOutputRefForward());
        #endif
            Smurf::CompileTime::foreachElemInTuple(fields, Utils::_WriteToStreamForward(os));
            return os;
        }
    };
}