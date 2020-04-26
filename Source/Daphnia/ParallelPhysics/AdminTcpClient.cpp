
#include "AdminTcpClient.h"
#include "AdminProtocol.h"
#include "ParallelPhysics.h"

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#undef TEXT
#include <windows.h>
#include <winsock2.h>
#include "iosfwd"
#undef min
#undef max
#pragma warning( disable : 4996)

namespace PPh
{
namespace AdminTcp
{
namespace
{
	WSADATA wsaData;
	SOCKET SendingSocket;
}
bool Connect()
{
	// Server/receiver address
	SOCKADDR_IN ServerAddr;
	// Server/receiver port to connect to
	unsigned int Port = ADMIN_TCP_PORT;
	int  RetCode;

	// Initialize Winsock version 2.2
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	printf("Client: Winsock DLL status is %s.\n", wsaData.szSystemStatus);

	// Create a new socket to make a client connection.
	// AF_INET = 2, The Internet Protocol version 4 (IPv4) address family, TCP protocol
	SendingSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SendingSocket == INVALID_SOCKET)
	{
		printf("Client: socket() failed! Error code: %d\n", WSAGetLastError());
		// Do the clean up
		WSACleanup();
		// Exit with error
		return false;
	}
	else
	{
		printf("Client: socket() is OK!\n");
	}
	// Set up a SOCKADDR_IN structure that will be used to connect
	// to a listening server.
	ServerAddr.sin_family = AF_INET;
	// Port no.
	ServerAddr.sin_port = htons(Port);
	// The IP address
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// Make a connection to the server with socket SendingSocket.
	RetCode = connect(SendingSocket, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr));
	if (RetCode != 0)
	{
		printf("Client: connect() failed! Error code: %d\n", WSAGetLastError());
		// Close the socket
		closesocket(SendingSocket);
		// Do the clean up
		WSACleanup();
		// Exit with error
		return false;
	}
	else
	{
		printf("Client: connect() is OK, got connected...\n");
		printf("Client: Ready for sending and/or receiving data...\n");
	}

	return true;
}

void LoadCrumbs()
{
	do
	{
		PPh::MsgAdminGetNextCrumb msg;
		if (send(SendingSocket, (const char*)&msg, sizeof(msg), 0) != SOCKET_ERROR)
		{
			char buffer[CommonParams::DEFAULT_BUFLEN];
			if (recv(SendingSocket, buffer, sizeof(buffer), 0) != SOCKET_ERROR)
			{
				if (const MsgAdminGetNextCrumbResponse *msgRcv = PPh::QueryMessage<MsgAdminGetNextCrumbResponse>(buffer))
				{
					if (msgRcv->m_color == EtherColor::ZeroColor)
					{
						break;
					}
					ParallelPhysics::GetInstance()->InitEtherCell(VectorInt32Math(msgRcv->m_posX, msgRcv->m_posY, msgRcv->m_posZ), EtherType::Crumb, msgRcv->m_color);
				}
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	} while (true);
}

void Disconnect()
{
	// Close the socket
	closesocket(SendingSocket);
	// Do the clean up
	WSACleanup();
}

} // namespace AdminTcp
} // namespace PPh
