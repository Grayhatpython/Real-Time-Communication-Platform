#pragma once

#include "engine/CommonType.h"

namespace network
{
    //  TEMP
    #pragma pack(push, 1)
    struct PacketHeader
    {
        uint16 size;
        uint16 id;
    };
    #pragma pack(pop)

}