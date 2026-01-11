#include "Pch.hpp"
#include "NetworkEvent.hpp"
#include "Session.hpp"
#include "Acceptor.hpp"

namespace servercore
{
   std::shared_ptr<Session> ConnectEvent::GetOwnerSession()
   {
       return std::static_pointer_cast<Session>(_owner);
   }

   std::shared_ptr<Session> DisconnectEvent::GetOwnerSession()
   {
       return std::static_pointer_cast<Session>(_owner);
   }
   
   std::shared_ptr<Acceptor> AcceptorEvent::GetOwnerAcceptor()
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