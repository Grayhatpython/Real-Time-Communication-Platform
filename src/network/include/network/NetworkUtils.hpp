#pragma once

#include "NetworkAddress.hpp"

namespace network
{
	class NetworkUtils
	{
	public:
		static void Initialize();
		static void Clear();

	public:
		static SocketFd	CreateSocketFd(bool overlapped = true);
		static void		CloseSocketFd(SocketFd& socket);

		static bool		Bind(SocketFd socket, NetworkAddress networkAddress);
		static bool		Bind(SocketFd socket, uint16 port);
		static bool		Listen(SocketFd socket, int32 backlog = SOMAXCONN);

	public:
		static bool 	SetNonBlocking(SocketFd socket);
		static bool		SetLinger(SocketFd socket, uint16 onoff, uint16 linger);
		static bool		SetReuseAddress(SocketFd socket, bool flag);
		static bool		SetTcpNoDelay(SocketFd socket, bool flag);
		static bool		SetRecvBufferSize(SocketFd socket, int32 size);
		static bool		SetSendBufferSize(SocketFd socket, int32 size);
	};

	template<typename T>
	static inline bool SetSocketOpt(SocketFd socket, int32 level, int32 optName, T optVal)
	{
		return RESULT_ERROR != ::setsockopt(socket, level, optName, &optVal, sizeof(T));
	}
}