#include "network/NetworkPch.h"
#include "network/NetworkEvent.h"
#include "network/Session.h"
#include "network/Acceptor.h"

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

   std::shared_ptr<Session> SendEvent::GetOwnerSession()
   {
       return std::static_pointer_cast<Session>(_owner);
   }

    std::shared_ptr<Session> RecvEvent::GetOwnerSession()
   {
      return std::static_pointer_cast<Session>(_owner);
   }
}