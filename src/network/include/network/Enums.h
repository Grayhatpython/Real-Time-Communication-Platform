#pragma once
#include "network/Types.h"

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

	enum class SessionState
	{
		ConnectPending,
		Connected,
		DisconnectPosted,
		Disconnected,
	};

	enum class CoreEventType : uint8
    {
        Wakeup,
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

		//	Successed
		NetworkEventDispatched,

		//	Recv Error
		InvalidSessionStateProcessRecv,
		GracefulClose,
		StreamBufferOverflow,
		InvalidPacketData,

		//	Send Error
		InvalidSessionStateProcessSend,
		
		InvalidNetworkEvent,
	};
}