#include "Pch.h"
#include "ServerSession.h"
#include "PacketDispatcher.h"

void ServerSession::OnConnected()
{
    auto session = shared_from_this();

    {
        //  register Test
        Protocol::C2S_Register registerPacket;
        registerPacket.username = "cisco";
        registerPacket.password = "cisco123";
        Protocol::SendPacket(session, Protocol::PacketId::C2S_Register, registerPacket);
    }
}

void ServerSession::OnDisconnected()
{

}

void ServerSession::OnRecv(BYTE *buffer, int32 numOfBytes)
{
    auto session = shared_from_this();

    engine::BinaryReader br(buffer,numOfBytes);

    uint16 size = 0;
    Protocol::PacketId id;

    br.Read(size);
    br.Read(id);

    network::PacketDispatcher::Dispatch(session, id, br);
}

void ServerSession::OnSend()
{

}
