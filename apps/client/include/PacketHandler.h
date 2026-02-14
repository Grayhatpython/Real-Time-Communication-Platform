#pragma once

class PacketHandler
{
public:
    static void RegisterPacketHandleFunc();

private:
    static void RegisterS2CAuthOkHandleFunc();
    static void RegisterS2CAuthFailHandleFunc();
};