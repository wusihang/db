#pragma once
#include <cstdint>
namespace IO {

enum class CompressionMethod {
    NONE = 0,
    LZ4 =  1
};

enum class CompressionMethodByte : uint8_t
{
    NONE     = 0x02,
    LZ4      = 0x82,
    ZSTD     = 0x90,
};

}
