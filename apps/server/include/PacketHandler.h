#pragma once

class DbWorker;
class PacketHandler
{
public:
    static void RegisterPacketHandleFunc(DbWorker* dbWorker);

private:
    static void RegisterCS2RegisterHandleFunc(DbWorker* dbWorker);
    static void RegisterCS2LoginHandleFunc(DbWorker* dbWorker);
    static void RegisterCS2ResumeHandleFunc(DbWorker* dbWorker);
};