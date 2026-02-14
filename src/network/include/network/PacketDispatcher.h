#pragma once

#include <memory>
#include <functional>
#include <unordered_map>

#include "network/Protocol.h"

#include "engine/BinaryReader.h"

namespace network
{
    class PacketDispatcher
    {
    public:
        using PacketHandleFunc = std::function<void(std::shared_ptr<network::Session>&, engine::BinaryReader&)>;

        template<typename PacketType>
        static void Register(Protocol::PacketId id, std::function<void(std::shared_ptr<network::Session>&, const PacketType&)> handleFunc)
        {
            S_packetIdToPacketHandleFuncMap[id] = [func = std::move(handleFunc)](std::shared_ptr<Session>& session, engine::BinaryReader& br) mutable {
                PacketType packet;
                if (packet.Deserialize(br) == false)
                {
                    session->Disconnect();
                    return;
                }
                
                func(session, packet);
            };
        }

        static void Dispatch(std::shared_ptr<network::Session>& session, Protocol::PacketId id, engine::BinaryReader& br);

    private:
        static std::unordered_map<Protocol::PacketId, PacketHandleFunc> S_packetIdToPacketHandleFuncMap;
    };
}