#pragma once
#include "Types.hpp"

namespace servercore
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

		ControlEventDispatched = 200
	};
}