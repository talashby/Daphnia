
#include "ParallelPhysics.h"
#include "vector"
#include "algorithm"
#include "array"
#include "thread"
#include "fstream"
#include "atomic"
#include "chrono"
#include "AdminProtocol.h"
#include "ServerProtocol.h"
#include "../MyPlayerController.h"

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#undef TEXT
#include <windows.h>
#include <winsock2.h>
#include "iosfwd"
#include "mutex"
#undef min
#undef max

#pragma warning( disable : 4996)

namespace PPh
{

ParallelPhysics *s_parallelPhysicsInstance = nullptr;

std::vector< std::vector< std::vector<struct EtherCell> > > s_universe;
std::atomic<int32_t> s_waitThreadsCount = 0; // thread synchronization variable
// stats
uint32_t s_quantumOfTimePerSecond = 0;
uint32_t s_universeThreadsNum = 0;
uint32_t s_TickTimeMusAverageUniverseThreadsMin = 0;
uint32_t s_TickTimeMusAverageUniverseThreadsMax = 0;
uint32_t s_TickTimeMusAverageObserverThread = 0;


struct Photon
{
	Photon() = default;
	explicit Photon(const OrientationVectorMath &orientation) : m_orientation(orientation)
	{}
	EtherColor m_color;
	OrientationVectorMath m_orientation;
	PhotonParam m_param;
};

struct EtherCell
{
	EtherCell() = default;
	explicit EtherCell(EtherType::EEtherType type) : m_type(type)
	{}
	int32_t m_type;
	EtherColor m_color;
	std::array <std::array<Photon, 26>, 2> m_photons;
};

bool ParallelPhysics::Init(const VectorInt32Math &universeSize)
{
	if (0 < universeSize.m_posX && 0 < universeSize.m_posY && 0 < universeSize.m_posZ)
	{
		s_universe.resize(universeSize.m_posX);
		for (auto &itY : s_universe)
		{
			itY.resize(universeSize.m_posY);
			for (auto &itZ : itY)
			{
				itZ.resize(0);
				EtherCell cell(EtherType::Space);
				cell.m_color = EtherColor::ZeroColor;
				{
					for (int ii = 0; ii < cell.m_photons[0].size(); ++ii)
					{
						cell.m_photons[0][ii].m_color.m_colorA = 0;
					}
				}
				{
					for (int ii = 0; ii < cell.m_photons[1].size(); ++ii)
					{
						cell.m_photons[1][ii].m_color.m_colorA = 0;
					}
				}
				itZ.resize(universeSize.m_posZ, cell);
			}
		}
		if (s_parallelPhysicsInstance)
		{
			delete s_parallelPhysicsInstance;
		}
		s_parallelPhysicsInstance = new ParallelPhysics();
		GetInstance()->m_universeSize = universeSize;
		return true;
	}
	return false;
}

bool ParallelPhysics::SaveUniverse(const std::string &fileName)
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


ParallelPhysics* ParallelPhysics::GetInstance()
{
	return s_parallelPhysicsInstance;
}

const VectorInt32Math & ParallelPhysics::GetUniverseSize() const
{
	return m_universeSize;
}

std::thread s_simulationThread;
void ParallelPhysics::StartSimulation()
{
	m_isSimulationRunning = true;
	static uint64_t lastObserverId = 0;
	SOCKET socketC = 0;
	u_short port = CLIENT_UDP_PORT_START;
	for (; port < CLIENT_UDP_PORT_START + MAX_CLIENTS; ++port)
	{
		struct sockaddr_in serverInfo;
		int len = sizeof(serverInfo);
		serverInfo.sin_family = AF_INET;
		serverInfo.sin_port = htons(port);
		serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
		socketC = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		DWORD timeout = 1000; // 1000 ms
		setsockopt(socketC, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(DWORD));
		MsgCheckVersion msg;
		msg.m_clientVersion = PROTOCOL_VERSION;
		msg.m_observerId = lastObserverId;
		if (sendto(socketC, (const char*)&msg, sizeof(msg), 0, (sockaddr*)&serverInfo, len) != SOCKET_ERROR)
		{
			char buffer[DEFAULT_BUFLEN];
			if (recvfrom(socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, &len) > 0)
			{
				if (const MsgCheckVersionResponse *msgReceive = QueryMessage<MsgCheckVersionResponse>(buffer))
				{
					if (msgReceive->m_serverVersion == PROTOCOL_VERSION)
					{
						lastObserverId = msgReceive->m_observerId;
						break;
					}
					else
					{
						// wrong protocol
						closesocket(socketC);
						socketC = 0;
						break;
					}
				}
			}
		}
		socketC = 0;
		closesocket(socketC);
	}
	if (socketC)
	{
		u_long mode = 1;  // 1 to enable non-blocking socket
		ioctlsocket(socketC, FIONBIO, &mode);
		s_simulationThread = std::thread([this, socketC, port]()
		{
			while (m_isSimulationRunning)
			{
				Observer::GetInstance()->PPhTick(socketC, port);
			}
			closesocket(socketC);
		});
	}
	else
	{
		int tt = 0;
		// server not found
	}

	//u_long mode = 1;  // 1 to enable non-blocking socket
	//ioctlsocket(socketC, FIONBIO, &mode);

}

void ParallelPhysics::StopSimulation()
{
	m_isSimulationRunning = false;
	if (s_simulationThread.native_handle())
	{
		s_simulationThread.join();
	}
}

bool ParallelPhysics::IsSimulationRunning() const
{
	return m_isSimulationRunning;
}

ParallelPhysics::ParallelPhysics()
{}

int32_t ParallelPhysics::GetCellPhotonIndex(const VectorInt32Math &unitVector)
{
	int32_t index = (unitVector.m_posX + 1) * 9 + (unitVector.m_posY + 1) * 3 + (unitVector.m_posZ + 1);
	if (index > 13)
	{
		--index;
	}
	return index;
}

bool ParallelPhysics::InitEtherCell(const VectorInt32Math &pos, EtherType::EEtherType type, const EtherColor &color)
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
				for (int ii = 0; ii < cell.m_photons[0].size(); ++ii)
				{
					Photon &photon = cell.m_photons[0][ii];
					photon.m_color = EtherColor::ZeroColor;
				}
				for (int ii = 0; ii < cell.m_photons[1].size(); ++ii)
				{
					Photon &photon = cell.m_photons[1][ii];
					photon.m_color = EtherColor::ZeroColor;
				}
				return true;
			}
		}
	}
	return false;
}

uint32_t ParallelPhysics::GetFPS()
{
	return s_quantumOfTimePerSecond;
}

uint32_t ParallelPhysics::GetTickTimeMusObserverThread()
{
	return s_TickTimeMusAverageObserverThread;
}

uint32_t ParallelPhysics::GetUniverseThreadsNum()
{
	return s_universeThreadsNum;
}

uint32_t ParallelPhysics::GetTickTimeMusUniverseThreadsMin()
{
	return s_TickTimeMusAverageUniverseThreadsMin;
}

uint32_t ParallelPhysics::GetTickTimeMusUniverseThreadsMax()
{
	return s_TickTimeMusAverageUniverseThreadsMax;
}

bool ParallelPhysics::GetNextCrumb(VectorInt32Math &outCrumbPos, EtherColor &outCrumbColor)
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

Observer* s_observer = nullptr;
std::mutex s_observerStateParamsMutex;

void Observer::Init()
{
	if (s_observer)
	{
		delete s_observer;
	}
	s_observer = new Observer();
}

PPh::Observer* Observer::GetInstance()
{
	return s_observer;
}

void Observer::PPhTick(uint64_t socketC, uint32_t port)
{
	struct sockaddr_in serverInfo;
	int len = sizeof(serverInfo);
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_port = htons(port);
	serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
	MsgGetState msg;
	if (sendto(socketC, (const char*)&msg, sizeof(msg), 0, (sockaddr*)&serverInfo, len) != SOCKET_ERROR)
	{
		if (GetTimeMs() - m_lastUpdateStateExtTime > UPDATE_EYE_TEXTURE_OUT)
		{
			m_lastUpdateStateExtTime = GetTimeMs();
			MsgGetStateExt msgGetStateExt;
			sendto(socketC, (const char*)&msgGetStateExt, sizeof(msgGetStateExt), 0, (sockaddr*)&serverInfo, len);
		}
		if (GetTimeMs() - m_lastStatisticRequestTime > STATISTIC_REQUEST_PERIOD)
		{
			m_lastStatisticRequestTime = GetTimeMs();
			MsgGetStatistics msgGetStatistics;
			sendto(socketC, (const char*)&msgGetStatistics, sizeof(msgGetStatistics), 0, (sockaddr*)&serverInfo, len);
		}

		AMyPlayerController *controller = AMyPlayerController::GetInstance();
		if (controller)
		{
			if (controller->IsLeft())
			{
				MsgRotateLeft msgMove;
				msgMove.m_value = 128;
				sendto(socketC, (const char*)&msgMove, sizeof(msgMove), 0, (sockaddr*)&serverInfo, len);
			}
			if (controller->IsRight())
			{
				MsgRotateRight msgMove;
				msgMove.m_value = 128;
				sendto(socketC, (const char*)&msgMove, sizeof(msgMove), 0, (sockaddr*)&serverInfo, len);
			}
			if (controller->IsUp())
			{
				MsgRotateDown msgMove;
				msgMove.m_value = 128;
				sendto(socketC, (const char*)&msgMove, sizeof(msgMove), 0, (sockaddr*)&serverInfo, len);
			}
			if (controller->IsDown())
			{
				MsgRotateUp msgMove;
				msgMove.m_value = 128;
				sendto(socketC, (const char*)&msgMove, sizeof(msgMove), 0, (sockaddr*)&serverInfo, len);
			}
			if (controller->IsForward())
			{
				MsgMoveForward msgMove;
				msgMove.m_value = 255;
				sendto(socketC, (const char*)&msgMove, sizeof(msgMove), 0, (sockaddr*)&serverInfo, len);
			}
			if (controller->IsBackward())
			{
				MsgMoveBackward msgMove;
				msgMove.m_value = 255;
				sendto(socketC, (const char*)&msgMove, sizeof(msgMove), 0, (sockaddr*)&serverInfo, len);
			}
		}

		char buffer[DEFAULT_BUFLEN];
		while(recvfrom(socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, &len) > 0)
		{
			static uint64_t time = 0;
			if (const MsgGetStateResponse *msgGetStateResponse = QueryMessage<MsgGetStateResponse>(buffer))
			{
				time = msgGetStateResponse->m_time;
			}
			else if (const MsgGetStateExtResponse *msgGetStateExtResponse = QueryMessage<MsgGetStateExtResponse>(buffer))
			{
				std::lock_guard<std::mutex> guard(s_observerStateParamsMutex);
				Observer::GetInstance()->m_latitude = msgGetStateExtResponse->m_latitude;
				Observer::GetInstance()->m_longitude = msgGetStateExtResponse->m_longitude;
				Observer::GetInstance()->m_position = msgGetStateExtResponse->m_pos;
				Observer::GetInstance()->m_movingProgress = msgGetStateExtResponse->m_movingProgress;
				if (Observer::GetInstance()->m_eatenCrumbNum < msgGetStateExtResponse->m_eatenCrumbNum)
				{
					Observer::GetInstance()->m_eatenCrumbNum = msgGetStateExtResponse->m_eatenCrumbNum;
					Observer::GetInstance()->m_eatenCrumbPos = msgGetStateExtResponse->m_eatenCrumbPos;
				}
			}
			else if (const MsgSendPhoton *msgSendPhoton = QueryMessage<MsgSendPhoton>(buffer))
			{
				// receive photons back // revert Y-coordinate because of texture format
				m_eyeColorArray[OBSERVER_EYE_SIZE - msgSendPhoton->m_posY - 1][msgSendPhoton->m_posX] = msgSendPhoton->m_color;
				m_eyeUpdateTimeArray[OBSERVER_EYE_SIZE - msgSendPhoton->m_posY - 1][msgSendPhoton->m_posX] = time;
			}
			else if (const MsgGetStatisticsResponse *msgRcv = QueryMessage<MsgGetStatisticsResponse>(buffer))
			{
				s_quantumOfTimePerSecond = msgRcv->m_fps;
				s_TickTimeMusAverageObserverThread = msgRcv->m_observerThreadTickTime;
				s_TickTimeMusAverageUniverseThreadsMin = msgRcv->m_universeThreadMinTickTime;
				s_TickTimeMusAverageUniverseThreadsMax = msgRcv->m_universeThreadMaxTickTime;
				s_universeThreadsNum = msgRcv->m_universeThreadsCount;
			}


			// update eye texture
			if (GetTimeMs() - m_lastTextureUpdateTime > UPDATE_EYE_TEXTURE_OUT)
			{
				m_lastTextureUpdateTime = GetTimeMs();
				SP_EyeColorArray spEyeColorArrayOut;
				spEyeColorArrayOut = std::atomic_load(&m_spEyeColorArrayOut);
				if (!spEyeColorArrayOut)
				{
					spEyeColorArrayOut = std::make_shared<EyeColorArray>();
					EyeColorArray &eyeColorArray = *spEyeColorArrayOut;
					for (int yy = 0; yy < eyeColorArray.size(); ++yy)
					{
						for (int xx = 0; xx < eyeColorArray[yy].size(); ++xx)
						{
							eyeColorArray[yy][xx] = m_eyeColorArray[yy][xx];
							int64_t timeDiff = time - m_eyeUpdateTimeArray[yy][xx];
							uint8_t alpha = m_eyeColorArray[yy][xx].m_colorA;
							if (timeDiff < EYE_IMAGE_DELAY)
							{
								alpha = alpha * (EYE_IMAGE_DELAY - timeDiff) / EYE_IMAGE_DELAY;
								eyeColorArray[yy][xx].m_colorA = alpha;
							}
							else
							{
								alpha = 0;
								//eyeColorArray[ii][jj] = EtherColor::ZeroColor;
								//eyeColorArray[ii][jj].m_colorA = 255;
							}
							eyeColorArray[yy][xx].m_colorA = alpha;
						}
					}
					std::atomic_store(&m_spEyeColorArrayOut, spEyeColorArrayOut);
				}
			}
		}
	}

	Sleep(1); // work imitation
}

void Observer::ChangeOrientation(const SP_EyeState &eyeState)
{
	std::atomic_store(&m_newEyeState, eyeState);
}

SP_EyeColorArray Observer::GrabTexture()
{
	SP_EyeColorArray spEyeColorArrayOut;
	std::atomic_store(&spEyeColorArrayOut, m_spEyeColorArrayOut);
	SP_EyeColorArray spEyeColorArrayEmpty;
	std::atomic_store(&m_spEyeColorArrayOut, spEyeColorArrayEmpty);
	return spEyeColorArrayOut;
}

PPh::VectorInt32Math Observer::GetPosition() const
{
	return m_position;
}

const VectorInt32Math& Observer::GetOrientMinChanger() const
{
	return m_orientMinChanger;
}

const VectorInt32Math& Observer::GetOrientMaxChanger() const
{
	return m_orientMaxChanger;
}

void Observer::GetStateParams(VectorInt32Math &outPosition, uint16_t &outMovingProgress, int16_t &outLatitude, int16_t &outLongitude, VectorInt32Math &outEatenCrumbPos)
{
	std::lock_guard<std::mutex> guard(s_observerStateParamsMutex);
	outPosition = m_position;
	outMovingProgress = m_movingProgress;
	outLatitude = m_latitude;
	outLongitude = m_longitude;
	outEatenCrumbPos = m_eatenCrumbPos;
	m_eatenCrumbPos = VectorInt32Math::ZeroVector;
}

int32_t RoundToMinMaxPPhInt(float value)
{
	int32_t result = 0;
	if (value < 0)
	{
		result = (int32_t)(value - 0.5f);
	}
	else
	{
		result = (int32_t)(value + 0.5f);
	}

	return result;
}

/*int32_t FixFloatErrors(int32_t component, int32_t maxComponentValue)
{
	int32_t componentCorrect = component;
	if (std::abs(component) == maxComponentValue)
	{
		if (0 > component)
		{
			componentCorrect = PPh::OrientationVectorMath::PPH_INT_MIN;
		}
		else
		{
			componentCorrect = PPh::OrientationVectorMath::PPH_INT_MAX;
		}
	}
	return componentCorrect;
}*/

OrientationVectorMath Observer::MaximizePPhOrientation(const VectorFloatMath &orientationVector)
{
	float maxComponent = std::max(std::max(std::abs(orientationVector.m_posX), std::abs(orientationVector.m_posY)), std::abs(orientationVector.m_posZ));
	double factor = 0;
	if (maxComponent > 0)
	{
		factor = OrientationVectorMath::PPH_INT_MAX / (double)maxComponent;
	}

	OrientationVectorMath pphOrientation(RoundToMinMaxPPhInt(orientationVector.m_posX*factor), RoundToMinMaxPPhInt(orientationVector.m_posY*factor),
		RoundToMinMaxPPhInt(orientationVector.m_posZ*factor));

	//int32_t maxPPhComponent = std::max(std::max(std::abs(pphOrientation.m_posX), std::abs(pphOrientation.m_posY)), std::abs(pphOrientation.m_posZ));
	//pphOrientation.m_posX = FixFloatErrors(pphOrientation.m_posX, maxPPhComponent);
	//pphOrientation.m_posY = FixFloatErrors(pphOrientation.m_posY, maxPPhComponent);
	//pphOrientation.m_posZ = FixFloatErrors(pphOrientation.m_posZ, maxPPhComponent);

	return pphOrientation;
}

void Observer::SetPosition(const VectorInt32Math &pos)
{
	m_position = pos;
}

void Observer::CalculateOrientChangers(const EyeArray &eyeArray)
{
	OrientationVectorMath orientMin(OrientationVectorMath::PPH_INT_MAX, OrientationVectorMath::PPH_INT_MAX, OrientationVectorMath::PPH_INT_MAX);
	OrientationVectorMath orientMax(OrientationVectorMath::PPH_INT_MIN, OrientationVectorMath::PPH_INT_MIN, OrientationVectorMath::PPH_INT_MIN);
	for (int ii = 0; ii < eyeArray.size(); ++ii)
	{
		for (int jj = 0; jj < eyeArray[ii].size(); ++jj)
		{
			orientMin.m_posX = std::min(orientMin.m_posX, eyeArray[ii][jj].m_posX);
			orientMin.m_posY = std::min(orientMin.m_posY, eyeArray[ii][jj].m_posY);
			orientMin.m_posZ = std::min(orientMin.m_posZ, eyeArray[ii][jj].m_posZ);
			orientMax.m_posX = std::max(orientMax.m_posX, eyeArray[ii][jj].m_posX);
			orientMax.m_posY = std::max(orientMax.m_posY, eyeArray[ii][jj].m_posY);
			orientMax.m_posZ = std::max(orientMax.m_posZ, eyeArray[ii][jj].m_posZ);
		}
	}

	m_orientMinChanger = VectorInt32Math(VectorInt32Math::PPH_INT_MAX, VectorInt32Math::PPH_INT_MAX, VectorInt32Math::PPH_INT_MAX);
	for (int ii = 0; ii < 3; ++ii)
	{
		if (0 <= orientMin.m_posArray[ii] && 0 <= orientMax.m_posArray[ii])
		{
			m_orientMinChanger.m_posArray[ii] = 0;
		}
	}

	m_orientMaxChanger = VectorInt32Math(VectorInt32Math::PPH_INT_MAX, VectorInt32Math::PPH_INT_MAX, VectorInt32Math::PPH_INT_MAX);
	for (int ii = 0; ii < 3; ++ii)
	{
		if (0 >= orientMin.m_posArray[ii] && 0 >= orientMax.m_posArray[ii])
		{
			m_orientMaxChanger.m_posArray[ii] = 0;
		}
	}
}

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
			char buffer[DEFAULT_BUFLEN];
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

}

}
