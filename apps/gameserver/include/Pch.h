#pragma once

#include <iostream>

#include "engine/CommonType.h"
#include "network/Types.h"

//  TEMP
#pragma pack(push, 1)
struct PacketHeader
{
    uint16 size;
    uint16 id;
};
#pragma pack(pop)