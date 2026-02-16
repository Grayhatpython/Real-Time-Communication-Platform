#include "Pch.h"
#include "ClientSession.h"
#include "PacketDispatcher.h"

void ClientSession::OnConnected() 
{
    auto clientSession = std::static_pointer_cast<ClientSession>(shared_from_this());
}

void ClientSession::OnDisconnected() 
{
    auto clientSession = std::static_pointer_cast<ClientSession>(shared_from_this());
}

void ClientSession::OnRecv(BYTE* buffer, int32 numOfBytes) 
{
    auto session = shared_from_this();

    engine::BinaryReader br(buffer,numOfBytes);

    uint16 size = 0;
    Protocol::PacketId id;

    br.Read(size);
    br.Read(id);

    network::PacketDispatcher::Dispatch(session, id, br);
}

void ClientSession::OnSend() 
{

}