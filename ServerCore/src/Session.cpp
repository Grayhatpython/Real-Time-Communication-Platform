#include "Pch.hpp"
#include "Session.hpp"

namespace servercore
{
    Session::~Session()
    {

    }

    NetworkObjectType Session::GetNetworkObjectType()
    {
        return NetworkObjectType::None;
    }

    SocketFd Session::GetSocketFd()
    {
        return INVALID_SOCKET_FD_VALUE;
    }

    void Session::Dispatch(INetworkEvent* networkEvent, bool succeeded, int32 errorCode)
    {

    }
}