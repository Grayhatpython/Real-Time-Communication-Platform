#pragma once

#include <iostream>
#include <vector>
#include <cstdint>
#include <string>

#include "engine/CommonType.h"
#include "network/Types.h"
#include "gamelogic/Types.h"

//  TEMP
#pragma pack(push, 1)
struct PacketHeader
{
    uint16 size;
    uint16 id;
};
#pragma pack(pop)