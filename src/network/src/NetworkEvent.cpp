#include "network/NetworkPch.hpp"
#include "network/NetworkEvent.hpp"
#include "network/Session.hpp"
#include "network/Acceptor.hpp"

namespace network
{
   std::shared_ptr<Session> ConnectEvent::GetOwnerSession()
   {
       return std::static_pointer_cast<Session>(_owner);
   }

   std::shared_ptr<Session> DisconnectEvent::GetOwnerSession()
   {
       return std::static_pointer_cast<Session>(_owner);
   }
   
   std::shared_ptr<Acceptor> AcceptEvent::GetOwnerAcceptor()
   {
       return std::static_pointer_cast<Acceptor>(_owner);
   }

   std::shared_ptr<Session> SendEvent::GetOwnerSession()
   {
       return std::static_pointer_cast<Session>(_owner);
   }

    std::shared_ptr<Session> RecvEvent::GetOwnerSession()
   {
      return std::static_pointer_cast<Session>(_owner);
   }
}