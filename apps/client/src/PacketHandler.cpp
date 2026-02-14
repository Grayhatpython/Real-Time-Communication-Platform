#include "Pch.h"
#include "PacketHandler.h"
#include "ServerSession.h"

#include "network/Protocol.h"
#include "network/PacketDispatcher.h"

void PacketHandler::RegisterPacketHandleFunc()
{
    RegisterS2CAuthOkHandleFunc();
    RegisterS2CAuthFailHandleFunc();
}

void PacketHandler::RegisterS2CAuthOkHandleFunc()
{
    network::PacketDispatcher::Register<Protocol::S2C_AuthOk>(Protocol::PacketId::S2C_AuthOk,
        [](std::shared_ptr<network::Session>& session, const Protocol::S2C_AuthOk& authOkPacket)
        {
            auto serverSession = std::static_pointer_cast<ServerSession>(session);

            serverSession->SetAuthed(authOkPacket.userId, authOkPacket.token);
        });
}

void PacketHandler::RegisterS2CAuthFailHandleFunc()
{
    network::PacketDispatcher::Register<Protocol::S2C_AuthFail>(Protocol::PacketId::S2C_AuthFail,
        [](std::shared_ptr<network::Session>& session, const Protocol::S2C_AuthFail& authFailPacket)
        {
        
        });
}
