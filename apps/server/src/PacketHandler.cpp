#include "Pch.h"
#include "PacketHandler.h"

#include "network/Session.h"
#include "network/PacketDispatcher.h"

void PacketHandler::RegisterPacketHandleFunc()
{
    network::PacketDispatcher::Register<Protocol::C2S_Login>(Protocol::PacketId::C2S_Login,
        [](std::shared_ptr<network::Session> session, const Protocol::C2S_Login& loginPacket)
        {
            
            // TODO: DB 스레드로 Login 요청 던지고, 완료되면 S2C_LoginOk 송신
        });

}