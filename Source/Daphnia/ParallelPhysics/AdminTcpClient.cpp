
#include "AdminTcpClient.h"
#include "AdminProtocol.h"
#include "ObserverClient.h"
#include "fstream"

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
namespace AdminUniverse
{
static std::vector< std::vector< std::vector<struct EtherCell> > > s_universe;
static VectorInt32Math s_universeSize = VectorInt32Math::ZeroVector;
static uint32_t s_universeScale = 1;

struct EtherCell
{
	EtherCell() = default;
	explicit EtherCell(EtherType::EEtherType type) : m_type(type)
	{}
	int32_t m_type;
	EtherColor m_color;
	AActor *m_crumbActor;
};

bool Init(const VectorInt32Math &universeSize)
{
	if (0 < universeSize.m_posX && 0 < universeSize.m_posY && 0 < universeSize.m_posZ)
	{
		s_universeSize = universeSize;
		s_universe.resize(universeSize.m_posX);
		for (auto &itY : s_universe)
		{
			itY.resize(universeSize.m_posY);
			for (auto &itZ : itY)
			{
				itZ.resize(0);
				EtherCell cell(EtherType::Space);
				cell.m_color = EtherColor::ZeroColor;
				cell.m_crumbActor = nullptr;
				itZ.resize(universeSize.m_posZ, cell);
			}
		}
		return true;
	}
	return false;
}

bool SaveUniverse(const std::string &fileName)
{
	std::ofstream myfile;
	myfile.open(fileName);
	if (myfile.is_open())
	{
		for (int32_t posX = 0; posX < s_universe.size(); ++posX)
		{
			for (int32_t posY = 0; posY < s_universe[posX].size(); ++posY)
			{
				for (int32_t posZ = 0; posZ < s_universe[posX][posY].size(); ++posZ)
				{
					myfile << (uint8_t)s_universe[posX][posY][posZ].m_type;
				}
			}
		}
		myfile.close();
		return true;
	}
	return false;
}

const VectorInt32Math& GetUniverseSize()
{
	return s_universeSize;
}

bool InitEtherCell(const VectorInt32Math &pos, EtherType::EEtherType type, const EtherColor &color)
{
	if (s_universe.size() > pos.m_posX)
	{
		if (s_universe[pos.m_posX].size() > pos.m_posY)
		{
			if (s_universe[pos.m_posX][pos.m_posY].size() > pos.m_posZ)
			{
				EtherCell &cell = s_universe[pos.m_posX][pos.m_posY][pos.m_posZ];
				cell.m_type = type;
				cell.m_color = color;
				return true;
			}
		}
	}
	return false;
}

bool GetNextCrumb(VectorInt32Math &outCrumbPos, EtherColor &outCrumbColor)
{
	static int32_t s_posX = 0;
	static int32_t s_posY = 0;
	static int32_t s_posZ = 0;
	bool bResult = false;
	for (; s_posX < s_universe.size(); ++s_posX, s_posY = 0)
	{
		for (; s_posY < s_universe[s_posX].size(); ++s_posY, s_posZ = 0)
		{
			for (; s_posZ < s_universe[s_posX][s_posY].size(); ++s_posZ)
			{
				if (bResult)
				{
					break;
				}
				auto &cell = s_universe[s_posX][s_posY][s_posZ];
				if (cell.m_type == EtherType::Crumb)
				{
					outCrumbPos = VectorInt32Math(s_posX, s_posY, s_posZ);
					outCrumbColor = cell.m_color;
					bResult = true;
				}
			}
			if (bResult)
			{
				break;
			}
		}
		if (bResult)
		{
			break;
		}
	}
	if (!bResult)
	{
		s_posX = 0;
		s_posY = 0;
		s_posZ = 0;
	}
	return bResult;
}

void EtherCellSetCrumbActor(const VectorInt32Math &pos, AActor *crumbActor)
{
	EtherCell &cell = s_universe[pos.m_posX][pos.m_posY][pos.m_posZ];
	cell.m_crumbActor = crumbActor;
}

AActor* EtherCellGetCrumbActor(const VectorInt32Math &pos)
{
	EtherCell &cell = s_universe[pos.m_posX][pos.m_posY][pos.m_posZ];
	return cell.m_crumbActor;
}

uint32_t GetUniverseScale()
{
	return s_universeScale;
}
} // namespace AdminUniverse

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

	DWORD timeout = 1000; // ms
	setsockopt(SendingSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	// Set up a SOCKADDR_IN structure that will be used to connect
	// to a listening server.
	ServerAddr.sin_family = AF_INET;
	// Port no.
	ServerAddr.sin_port = htons(Port);
	// The IP address
	ServerAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

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

bool CheckVersion(uint32_t &serverVersion)
{
	PPh::MsgAdminCheckVersion msg;
	msg.m_clientVersion = ADMIN_PROTOCOL_VERSION;
	send(SendingSocket, (const char*)&msg, sizeof(msg), 0);
	char buffer[CommonParams::DEFAULT_BUFLEN];
	if (recv(SendingSocket, buffer, sizeof(buffer), 0) != SOCKET_ERROR)
	{
		if (const MsgAdminCheckVersionResponse *msgRcv = PPh::QueryMessage<MsgAdminCheckVersionResponse>(buffer))
		{
			serverVersion = msgRcv->m_serverVersion;
			if (msgRcv->m_serverVersion == ADMIN_PROTOCOL_VERSION)
			{
				AdminUniverse::s_universeScale = msgRcv->m_universeScale;
				return true;
			}
		}
	}
	return false;
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
					AdminUniverse::InitEtherCell(VectorInt32Math(msgRcv->m_posX, msgRcv->m_posY, msgRcv->m_posZ), EtherType::Crumb, msgRcv->m_color);
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

void RegisterAdminObserver(uint64_t observerId)
{
	PPh::MsgRegisterAdminObserver msg;
	msg.m_adminObserverId = observerId;
	send(SendingSocket, (const char*)&msg, sizeof(msg), 0);
}


} // namespace AdminTcp
} // namespace PPh
