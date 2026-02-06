#pragma once
#include "network/Types.hpp"

namespace network
{
	enum class NetworkEventType : uint8
	{
		None,
		Connect,
		Disconnect,
		Accept,
		Recv,
		Send,
		Error
	};

	//	TEMP : linux
	enum class NetworkObjectType 
	{
		None,
		Acceptor,
		Session,
	};

	enum class SessionState
	{
		ConnectPending,
		Connected,
		DisconnectPosted,
		Disconnected,
	};

	enum class CoreEventType : uint8
    {
        SessionRemove, 
        CoreShutdown
    };

	enum class DispatchResult
	{
		//	결과를 더 세부적으로..
		EventTriggered = 100,
		Timeout,
		Interrupted,
		ExitRequested,
		FatalError,
		NetworkEventDispatched,
		InvalidDispatcher,

		CoreEventDispatched = 200
	};
}