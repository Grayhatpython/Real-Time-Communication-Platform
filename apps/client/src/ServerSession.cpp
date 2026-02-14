#include "Pch.h"
#include "ServerSession.h"

#include "engine/BinaryReader.h"
#include "network/Protocol.h"
#include "network/PacketDispatcher.h"

void ServerSession::OnConnected()
{

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
