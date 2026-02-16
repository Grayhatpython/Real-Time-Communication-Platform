#include "Pch.h"
#include "PacketHandler.h"
#include "ServerSession.h"

#include "Protocol.h"
#include "PacketDispatcher.h"

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

            if(authOkPacket.type == AuthOKType::Register)
            {

            }
            else if(authOkPacket.type == AuthOKType::Login)
            {

            }
            else  if(authOkPacket.type ==  AuthOKType::Resume)
            {

            }
            else
            {
                //  TEMP
            }
            
            EN_LOG_DEBUG("S2C_AuthOk");
        });
}

void PacketHandler::RegisterS2CAuthFailHandleFunc()
{
    network::PacketDispatcher::Register<Protocol::S2C_AuthFail>(Protocol::PacketId::S2C_AuthFail,
        [](std::shared_ptr<network::Session>& session, const Protocol::S2C_AuthFail& authFailPacket)
        {
        
        });
}
