/* Log/FastLogStream.cpp: October 2013 reconstruction (Project OGRE).
   Routes FLog's binary log (FLog::Detail::SetBinaryLog) to a file or a TCP socket.
   Evidence: the 2013 Log.dll exports SetLogStreamFile 0x10009880, SetLogStreamNetworkClient
   0x100098d0 and SetLogStreamNetworkServer 0x100099c0, and holds their four messages
   ("LogStream: Dumping to file %s", "LogStream: Connecting to %s:%d", "LogStream: Dumping to
   socket", "LogStream: Listening for incoming connection on port %d"). Read from those bodies:
   fopen_s(..., "wb") then printf and SetBinaryLog(file, writer at 0x10009840); WSAStartup(0x101),
   socket(2, 1, 6), a zeroed sockaddr_in with family 2, inet_addr/htons, connect or
   bind+listen(1)+accept, closesocket on failure; the socket writer is at 0x10009860. Log.dll is
   built /GL (the Rich header counts this file among its two LTCG objects), so the bodies above are
   whole-program code. The 2013 Log.dll ships on Windows only: the POSIX branches 2016 adds for
   Mac and Android are dropped. */
#include "FastLogStream.h"
#include <stdio.h>

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

namespace {

// Binary log sinks: context is the FILE* or the SOCKET.
void writeToFile(void* context, const void* data, size_t size)
{
	fwrite(data, 1, size, static_cast<FILE*>(context));
}

void writeToSocket(void* context, const void* data, size_t size)
{
	send(static_cast<SOCKET>(reinterpret_cast<long>(context)), static_cast<const char*>(data), (int)size, 0);
}

// A TCP socket after Winsock 1.1 start-up, or INVALID_SOCKET.
SOCKET openTcpSocket()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(1, 1), &wsa);
	return socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
}

}

namespace FLog {

bool SetLogStreamFile(const char* path)
{
	FILE* file;
	if (fopen_s(&file, path, "wb") != 0)
		return false;

	printf("LogStream: Dumping to file %s\n", path);
	Detail::SetBinaryLog(file, writeToFile);
	return true;
}

bool SetLogStreamNetworkClient(const char* address, unsigned int port)
{
	SOCKET sock = openTcpSocket();
	if (sock == INVALID_SOCKET)
		return false;

	sockaddr_in server = {};
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(address);
	server.sin_port = htons(port);

	printf("LogStream: Connecting to %s:%d\n", address, port);
	if (connect(sock, reinterpret_cast<sockaddr*>(&server), sizeof(server)) >= 0)
	{
		printf("LogStream: Dumping to socket\n");
		Detail::SetBinaryLog(reinterpret_cast<void*>(sock), writeToSocket);
		return true;
	}

	closesocket(sock);
	return false;
}

bool SetLogStreamNetworkServer(unsigned int port)
{
	SOCKET listener = openTcpSocket();
	if (listener == INVALID_SOCKET)
		return false;

	sockaddr_in local = {};
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(port);

	// one client at a time; this call blocks until it connects
	if (bind(listener, reinterpret_cast<sockaddr*>(&local), sizeof(local)) >= 0 && listen(listener, 1) >= 0)
	{
		printf("LogStream: Listening for incoming connection on port %d\n", port);
		SOCKET client = accept(listener, NULL, NULL);
		if (client != INVALID_SOCKET)
		{
			printf("LogStream: Dumping to socket\n");
			Detail::SetBinaryLog(reinterpret_cast<void*>(client), writeToSocket);
			return true;
		}
	}

	closesocket(listener);
	return false;
}

}
