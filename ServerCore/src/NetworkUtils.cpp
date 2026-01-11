#include "Pch.hpp"
#include "NetworkUtils.hpp"

namespace servercore
{
	void NetworkUtils::Initialize()
	{

		//	클라이언트가 비정상적인 종료나, 네트워크 연결이 갑자기 끊어졌는 상황에 서버는 아직 그 상황을 감지못하고 해당 클라이언트에게 데이터를 보내기 위해
		//	send()또는 write() 호출 시 리눅스 커널을 상대방이 없는 상황인데 데이터를 쓰려고 하네? 라는 비정상적인 상황에 대해 해당 서버 프로세스에게 SIGPIPE 신호를 보낸다.
		//	SIGPIPE 신호 -> 서버 프로세스 강제 종료 -> 나머지 정상적인 연결의 클라이언트들을 서비스하던 서버 전체가 다운?
		//	그래서 커널에게 SIGPIPE 신호가 오면, 그냥 무시해라~
		//	위 상황에서 서버가 send() 호출 시 커널이 SIGPIPE 신호를 보내면 무시하고 대신 send() or write() 함수가 즉시 실패하고 -1을 반환한다
		//	이때 전역 변수 errno에는 EPIPE 라는 에러 코드가 설정된다.
		//	이제 서버는 프로세스가 죽는 대신, errno 값을 통해, 이 클라이언트와의 연결이 끊어졌구나 라는 사실을 감지하고 disconnect 과정을 하면된다.
		::signal(SIGPIPE, SIG_IGN);
	}

	void NetworkUtils::Clear()
	{

	}

	SocketFd NetworkUtils::CreateSocketFd(bool overlapped)
	{
		SocketFd socket = INVALID_SOCKET_FD_VALUE;

		if(overlapped == true)
		{
			socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			SetNonBlocking(socket);
		}
		else
		{
			socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		}

		return socket;
	}

	void NetworkUtils::CloseSocketFd(SocketFd& socket)
	{
		if (socket != INVALID_SOCKET_FD_VALUE)
		{
			::close(socket);
		}

		socket = INVALID_SOCKET_FD_VALUE;
	}

	bool NetworkUtils::Bind(SocketFd socket, NetworkAddress networkAddress)
	{
		return RESULT_ERROR != ::bind(socket, reinterpret_cast<struct sockaddr*>(&networkAddress.GetSocketAddress()), sizeof(struct sockaddr_in));
	}

	bool NetworkUtils::Bind(SocketFd socket, uint16 port)
	{
		struct sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = ::htonl(INADDR_ANY);
		address.sin_port = ::htons(port);

		return RESULT_ERROR != ::bind(socket, reinterpret_cast<const struct sockaddr*>(&address), sizeof(address));
	}

	bool NetworkUtils::Listen(SocketFd socket, int32 backlog)
	{
		return RESULT_ERROR != ::listen(socket, backlog);
	}

    bool NetworkUtils::SetNonBlocking(SocketFd socket)
    {
		if (socket != INVALID_SOCKET_FD_VALUE)
		{
			int32 flags = ::fcntl(socket, F_GETFL, 0);
			if(flags < 0)
            	return false; 
				
			if(::fcntl(socket, F_SETFL, flags | O_NONBLOCK) < 0)
            	return false;
		}

		return true;
    }

    bool NetworkUtils::SetLinger(SocketFd socket, uint16 onoff, uint16 linger)
    {
		struct linger opt;
		opt.l_onoff = onoff;
		opt.l_linger = linger;
		return SetSocketOpt(socket, SOL_SOCKET, SO_LINGER, opt);
	}

	bool NetworkUtils::SetReuseAddress(SocketFd socket, bool flag)
	{
		int optVal = flag ? 1 : 0;
		return SetSocketOpt(socket, SOL_SOCKET, SO_REUSEADDR, optVal);
	}

	bool NetworkUtils::SetTcpNoDelay(SocketFd socket, bool flag)
	{
		int optVal = flag ? 1 : 0;
		return SetSocketOpt(socket, IPPROTO_TCP, TCP_NODELAY, optVal);
	}

	bool NetworkUtils::SetRecvBufferSize(SocketFd socket, int32 size)
	{
		return SetSocketOpt(socket, SOL_SOCKET, SO_RCVBUF, size);
	}

	bool NetworkUtils::SetSendBufferSize(SocketFd socket, int32 size)
	{
		return SetSocketOpt(socket, SOL_SOCKET, SO_SNDBUF, size);
	}

}