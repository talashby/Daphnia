
#include "ObserverClient.h"
#include "vector"
#include "algorithm"
#include "array"
#include "thread"
#include "atomic"
#include "chrono"

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#undef TEXT
#include <windows.h>
#include <winsock2.h>
#include "mutex"
#include "AdminTcpClient.h"
#undef min
#undef max

#pragma warning( disable : 4996)
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

namespace PPh
{

ObserverClient* s_observer = nullptr;
std::mutex s_observerStateParamsMutex;
std::mutex s_serverStatisticsMutex;

bool s_wsaInitialised = false;
WSADATA s_wsaData;
uint64_t s_lastObserverId = 0;


void ObserverClient::Init(ObserverClient *observer)
{
	if (s_observer)
	{
		delete s_observer;
	}
	if (observer)
	{
		s_observer = observer;
	}
	else
	{
		s_observer = new ObserverClient();
	}
}

PPh::ObserverClient* ObserverClient::Instance()
{
	return s_observer;
}

std::thread s_simulationThread;
void ObserverClient::StartSimulation()
{
	// Initialize Winsock version 2.2
	bool wsaResult = WSAStartup(MAKEWORD(2, 2), &s_wsaData);
	s_wsaInitialised = 0 == wsaResult;
	printf("Client: Winsock DLL status is %s.\n", s_wsaData.szSystemStatus);

	m_isSimulationRunning = true;
	m_socketC = 0;
	m_port = CommonParams::CLIENT_UDP_PORT_START;
	for (; m_port < CommonParams::CLIENT_UDP_PORT_START + CommonParams::MAX_CLIENTS; ++m_port)
	{
		struct sockaddr_in serverInfo;
		int len = sizeof(serverInfo);
		serverInfo.sin_family = AF_INET;
		serverInfo.sin_port = htons(m_port);
		serverInfo.sin_addr.s_addr = inet_addr(SERVER_IP);
		m_socketC = (uint32_t)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		DWORD timeout = 1000; // 1000 ms
		setsockopt(m_socketC, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(DWORD));
		MsgCheckVersion msg;
		msg.m_clientVersion = CommonParams::PROTOCOL_VERSION;
		msg.m_observerId = s_lastObserverId;
		msg.m_observerType = static_cast<uint8_t>(OBSERVER_TYPE);
		if (sendto(m_socketC, (const char*)&msg, sizeof(msg), 0, (sockaddr*)&serverInfo, len) != SOCKET_ERROR)
		{
			char buffer[CommonParams::DEFAULT_BUFLEN];
			if (recvfrom(m_socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, &len) > 0)
			{
				if (const MsgCheckVersionResponse *msgReceive = QueryMessage<MsgCheckVersionResponse>(buffer))
				{
					if (msgReceive->m_serverVersion == CommonParams::PROTOCOL_VERSION)
					{
						s_lastObserverId = msgReceive->m_observerId; // in future s_lastObserverId can be stored to disk and used after restart client
						break;
					}
					else
					{
						// wrong protocol
						closesocket(m_socketC);
						m_socketC = 0;
						break;
					}
				}
			}
		}
		m_socketC = 0;
		closesocket(m_socketC);
	}
	if (m_socketC)
	{
		u_long mode = 1;  // 1 to enable non-blocking socket
		ioctlsocket(m_socketC, FIONBIO, &mode);
		s_simulationThread = std::thread([this]()
		{
			while (m_isSimulationRunning)
			{
				ObserverClient::Instance()->PPhTick();
			}
			closesocket(m_socketC);
		});
	}
	else
	{
		// server not found
	}

}

void ObserverClient::StopSimulation()
{
	if (s_wsaInitialised)
	{
		WSACleanup();
		s_wsaInitialised = false;
	}
	m_isSimulationRunning = false;
	if (s_simulationThread.native_handle())
	{
		s_simulationThread.join();
	}
}

bool ObserverClient::IsSimulationRunning() const
{
	return m_isSimulationRunning;
}

uint64_t ObserverClient::GetLastObserverID() const
{
	return s_lastObserverId;
}

void ObserverClient::PPhTick()
{
	MsgGetState msg;
	if (SendServerMsg(msg, sizeof(msg)))
	{
		if (GetTimeMs() - m_lastUpdateStateExtTime > 20) // get position/orientation data every n milliseconds
		{
			m_lastUpdateStateExtTime = GetTimeMs();
			MsgGetStateExt msgGetStateExt;
			SendServerMsg(msgGetStateExt, sizeof(msgGetStateExt));
		}
		if (GetTimeMs() - m_lastStatisticRequestTime > STATISTIC_REQUEST_PERIOD)
		{
			m_lastStatisticRequestTime = GetTimeMs();
			MsgGetStatistics msgGetStatistics;
			SendServerMsg(msgGetStatistics, sizeof(msgGetStatistics));
		}

		if (m_isLeft)
		{
			MsgRotateLeft msgMove;
			msgMove.m_value = 16;
			SendServerMsg(msgMove, sizeof(msgMove));
		}
		if (m_isRight)
		{
			MsgRotateRight msgMove;
			msgMove.m_value = 16;
			SendServerMsg(msgMove, sizeof(msgMove));
		}
		if (m_isUp)
		{
			MsgRotateDown msgMove;
			msgMove.m_value = 16;
			SendServerMsg(msgMove, sizeof(msgMove));
		}
		if (m_isDown)
		{
			MsgRotateUp msgMove;
			msgMove.m_value = 16;
			SendServerMsg(msgMove, sizeof(msgMove));
		}
		if (m_isForward)
		{
			MsgMoveForward msgMove;
			msgMove.m_value = 32 * PPh::AdminUniverse::GetUniverseScale();
			SendServerMsg(msgMove, sizeof(msgMove));
		}
		if (m_isBackward)
		{
			MsgMoveBackward msgMove;
			msgMove.m_value = 32 * PPh::AdminUniverse::GetUniverseScale();
			SendServerMsg(msgMove, sizeof(msgMove));
		}

		static uint64_t timeOfTheUniverse = 0;
		while(const char *buffer = RecvServerMsg())
		{
			if (const MsgGetStateResponse *msgGetStateResponse = QueryMessage<MsgGetStateResponse>(buffer))
			{
				timeOfTheUniverse = msgGetStateResponse->m_time;
			}
			else if (const MsgGetStateExtResponse *msgGetStateExtResponse = QueryMessage<MsgGetStateExtResponse>(buffer))
			{
				std::lock_guard<std::mutex> guard(s_observerStateParamsMutex);
				m_latitude = msgGetStateExtResponse->m_latitude;
				m_longitude = msgGetStateExtResponse->m_longitude;
				m_position = msgGetStateExtResponse->m_pos;
				m_movingProgress = msgGetStateExtResponse->m_movingProgress;
				if (m_eatenCrumbNum < msgGetStateExtResponse->m_eatenCrumbNum)
				{
					m_eatenCrumbNum = msgGetStateExtResponse->m_eatenCrumbNum;
					m_eatenCrumbPos = msgGetStateExtResponse->m_eatenCrumbPos;
					m_isEatenCrumb = true;
				}
			}
			else if (const MsgSendPhoton *msgSendPhoton = QueryMessage<MsgSendPhoton>(buffer))
			{
				// receive photons back // revert Y-coordinate because of texture format
				// photon (x,y) placed to [GetObserverEyeSize() - y -1][x] for simple copy to texture purpose
				m_eyeColorArray[GetObserverEyeSize() - msgSendPhoton->m_posY - 1][msgSendPhoton->m_posX] = msgSendPhoton->m_color;
				m_eyeUpdateTimeArray[GetObserverEyeSize() - msgSendPhoton->m_posY - 1][msgSendPhoton->m_posX] = timeOfTheUniverse;
			}
			else if (const MsgGetStatisticsResponse *msgRcv = QueryMessage<MsgGetStatisticsResponse>(buffer))
			{
				std::lock_guard<std::mutex> guard(s_serverStatisticsMutex);
				m_quantumOfTimePerSecond = msgRcv->m_fps;
				m_TickTimeMusAverageObserverThread = msgRcv->m_observerThreadTickTime;
				m_TickTimeMusAverageUniverseThreadsMin = msgRcv->m_universeThreadMinTickTime;
				m_TickTimeMusAverageUniverseThreadsMax = msgRcv->m_universeThreadMaxTickTime;
				m_universeThreadsNum = msgRcv->m_universeThreadsCount;
				m_clientServerPerformanceRatio = msgRcv->m_clientServerPerformanceRatio;
				m_serverClientPerformanceRatio = msgRcv->m_serverClientPerformanceRatio;
			}
			else
			{
				HandleReceivedMessage(buffer);
			}
		}

		// update eye texture
		if (timeOfTheUniverse && GetTimeMs() - m_lastTextureUpdateTime > UPDATE_EYE_TEXTURE_OUT)
		{
			m_lastTextureUpdateTime = GetTimeMs();
			SP_EyeColorArray spEyeColorArrayOut;
			spEyeColorArrayOut = std::atomic_load(&m_spEyeColorArrayOut);
			if (!spEyeColorArrayOut)
			{
				spEyeColorArrayOut = std::make_shared<EyeColorArray>();
				EyeColorArray &eyeColorArray = *spEyeColorArrayOut;
				for (uint32_t yy = 0; yy < eyeColorArray.size(); ++yy)
				{
					for (uint32_t xx = 0; xx < eyeColorArray[yy].size(); ++xx)
					{
						eyeColorArray[yy][xx] = m_eyeColorArray[yy][xx];
						int64_t timeDiff = timeOfTheUniverse - m_eyeUpdateTimeArray[yy][xx];
						uint8_t alpha = m_eyeColorArray[yy][xx].m_colorA;
						if (timeDiff < EYE_IMAGE_DELAY)
						{
							alpha = (uint8_t)(alpha * (EYE_IMAGE_DELAY - timeDiff) / EYE_IMAGE_DELAY);
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

	int64_t timeEllapsed;
	int64_t timeStart;
	int64_t timeDelta;

	QueryPerformanceFrequency((LARGE_INTEGER*)(&timeDelta));

	int64_t timeToWait = (int64_t)((double)timeDelta * (double)30 / 1000000.0);

	QueryPerformanceCounter((LARGE_INTEGER*)(&timeStart));

	timeEllapsed = timeStart;

	while ((timeEllapsed - timeStart) < timeToWait)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)(&timeEllapsed));

	};
	//Sleep(1); // work imitation
}

SP_EyeColorArray ObserverClient::GrabTexture()
{
	SP_EyeColorArray spEyeColorArrayOut;
	std::atomic_store(&spEyeColorArrayOut, m_spEyeColorArrayOut);
	SP_EyeColorArray spEyeColorArrayEmpty;
	std::atomic_store(&m_spEyeColorArrayOut, spEyeColorArrayEmpty);
	return spEyeColorArrayOut;
}

PPh::VectorInt32Math ObserverClient::GetPosition() const
{
	return m_position;
}

const VectorInt32Math& ObserverClient::GetOrientMinChanger() const
{
	return m_orientMinChanger;
}

const VectorInt32Math& ObserverClient::GetOrientMaxChanger() const
{
	return m_orientMaxChanger;
}

void ObserverClient::GetStateExtParams(VectorInt32Math &outPosition, uint16_t &outMovingProgress, int16_t &outLatitude,
	int16_t &outLongitude, bool &outIsEatenCrumb) const
{
	std::lock_guard<std::mutex> guard(s_observerStateParamsMutex);
	outPosition = m_position;
	outMovingProgress = m_movingProgress;
	outLatitude = m_latitude;
	outLongitude = m_longitude;
	outIsEatenCrumb = m_isEatenCrumb;
}

VectorInt32Math ObserverClient::GrabEatenCrumbPos()
{
	std::lock_guard<std::mutex> guard(s_observerStateParamsMutex);
	if (m_isEatenCrumb)
	{
		m_isEatenCrumb = false;
		return m_eatenCrumbPos;
	}
	return VectorInt32Math::ZeroVector;
}

void ObserverClient::GetStatisticsParams(uint32_t &outQuantumOfTimePerSecond, uint32_t &outUniverseThreadsNum,
	uint32_t &outTickTimeMusAverageUniverseThreadsMin, uint32_t &outTickTimeMusAverageUniverseThreadsMax,
	uint32_t &outTickTimeMusAverageObserverThread, uint64_t &outClientServerPerformanceRatio,
	uint64_t &outServerClientPerformanceRatio)
{
	std::lock_guard<std::mutex> guard(s_serverStatisticsMutex);
	outQuantumOfTimePerSecond = m_quantumOfTimePerSecond;
	outUniverseThreadsNum = m_universeThreadsNum;
	outTickTimeMusAverageUniverseThreadsMin = m_TickTimeMusAverageUniverseThreadsMin;
	outTickTimeMusAverageUniverseThreadsMax = m_TickTimeMusAverageUniverseThreadsMax;
	outTickTimeMusAverageObserverThread = m_TickTimeMusAverageObserverThread;
	outClientServerPerformanceRatio = m_clientServerPerformanceRatio;
	outServerClientPerformanceRatio = m_serverClientPerformanceRatio;
}

const char* ObserverClient::RecvServerMsg()
{
	static char buffer[CommonParams::DEFAULT_BUFLEN];
	struct sockaddr_in fromCur;
	int fromlen = sizeof(sockaddr_in);
	int result = recvfrom(m_socketC, buffer, sizeof(buffer), 0, (sockaddr*)&fromCur, &fromlen);
	if (result > 0)
	{
		return &buffer[0];
	}

	return nullptr;
}

bool ObserverClient::SendServerMsg(const MsgBase &msg, int32_t msgSize)
{
	struct sockaddr_in serverInfo;
	int len = sizeof(serverInfo);
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_port = htons(m_port);
	serverInfo.sin_addr.s_addr = inet_addr(SERVER_IP);
	if (sendto(m_socketC, msg.GetBuffer(), msgSize, 0, (sockaddr*)&serverInfo, len) == SOCKET_ERROR)
	{
		printf("SendServerMsg sendto error");
		return false;
	}
	return true;
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

void ObserverClient::HandleReceivedMessage(const char *buffer)
{
	printf("Unhandled message from server: %d", buffer[0]);
}

}
