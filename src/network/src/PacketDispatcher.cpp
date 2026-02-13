#include "network/NetworkPch.h"
#include "network/PacketDispatcher.h"

namespace network
{
    std::unordered_map<Protocol::PacketId, PacketDispatcher::PacketHandleFunc> PacketDispatcher::S_packetIdToPacketHandleFuncMap;

    void PacketDispatcher::Dispatch(std::shared_ptr<network::Session> session, Protocol::PacketId id, engine::BinaryReader& br)
    {
        auto findIt = S_packetIdToPacketHandleFuncMap.find(id);
        if (findIt == S_packetIdToPacketHandleFuncMap.end())
        {
            session->Disconnect();
            return;
        }

        findIt->second(std::move(session), br);
    }
}