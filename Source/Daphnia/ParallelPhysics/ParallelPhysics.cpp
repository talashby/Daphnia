
#include "ParallelPhysics.h"
#include "vector"
#include "algorithm"
#include "array"
#include "thread"
#include "fstream"
#include "atomic"
#include "chrono"
#include "../MyPlayerController.h"

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#undef TEXT
#include <windows.h>
#include <winsock2.h>
#include "mutex"
#undef min
#undef max

#pragma warning( disable : 4996)

namespace PPh
{

ParallelPhysics *s_parallelPhysicsInstance = nullptr;

std::vector< std::vector< std::vector<struct EtherCell> > > s_universe;

struct EtherCell
{
	EtherCell() = default;
	explicit EtherCell(EtherType::EEtherType type) : m_type(type)
	{}
	int32_t m_type;
	EtherColor m_color;
	AActor *m_crumbActor;
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
				cell.m_crumbActor = nullptr;
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

ParallelPhysics::ParallelPhysics()
{}

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
				return true;
			}
		}
	}
	return false;
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

void ParallelPhysics::EtherCellSetCrumbActor(const VectorInt32Math &pos, AActor *crumbActor)
{
	EtherCell &cell = s_universe[pos.m_posX][pos.m_posY][pos.m_posZ];
	cell.m_crumbActor = crumbActor;
}

AActor* ParallelPhysics::EtherCellGetCrumbActor(const VectorInt32Math &pos)
{
	EtherCell &cell = s_universe[pos.m_posX][pos.m_posY][pos.m_posZ];
	return cell.m_crumbActor;
}

Observer* s_observer = nullptr;
std::mutex s_observerStateParamsMutex;
std::mutex s_serverStatisticsMutex;

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

std::thread s_simulationThread;
void Observer::StartSimulation()
{
	m_isSimulationRunning = true;
	static uint64_t lastObserverId = 0;
	m_socketC = 0;
	m_port = CommonParams::CLIENT_UDP_PORT_START;
	for (; m_port < CommonParams::CLIENT_UDP_PORT_START + CommonParams::MAX_CLIENTS; ++m_port)
	{
		struct sockaddr_in serverInfo;
		int len = sizeof(serverInfo);
		serverInfo.sin_family = AF_INET;
		serverInfo.sin_port = htons(m_port);
		serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
		m_socketC = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		DWORD timeout = 1000; // 1000 ms
		setsockopt(m_socketC, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&timeout), sizeof(DWORD));
		MsgCheckVersion msg;
		msg.m_clientVersion = CommonParams::PROTOCOL_VERSION;
		msg.m_observerId = lastObserverId;
		if (sendto(m_socketC, (const char*)&msg, sizeof(msg), 0, (sockaddr*)&serverInfo, len) != SOCKET_ERROR)
		{
			char buffer[CommonParams::DEFAULT_BUFLEN];
			if (recvfrom(m_socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, &len) > 0)
			{
				if (const MsgCheckVersionResponse *msgReceive = QueryMessage<MsgCheckVersionResponse>(buffer))
				{
					if (msgReceive->m_serverVersion == CommonParams::PROTOCOL_VERSION)
					{
						lastObserverId = msgReceive->m_observerId;
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
				Observer::GetInstance()->PPhTick();
			}
			closesocket(m_socketC);
		});
	}
	else
	{
		// server not found
	}

}

void Observer::StopSimulation()
{
	m_isSimulationRunning = false;
	if (s_simulationThread.native_handle())
	{
		s_simulationThread.join();
	}
}

bool Observer::IsSimulationRunning() const
{
	return m_isSimulationRunning;
}

void Observer::PPhTick()
{
	MsgGetState msg;
	if (SendServerMsg(msg, sizeof(msg)))
	{
		if (GetTimeMs() - m_lastUpdateStateExtTime > UPDATE_EYE_TEXTURE_OUT)
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

		AMyPlayerController *controller = AMyPlayerController::GetInstance();
		if (controller)
		{
			if (controller->IsLeft())
			{
				MsgRotateLeft msgMove;
				msgMove.m_value = 4;
				SendServerMsg(msgMove, sizeof(msgMove));
			}
			if (controller->IsRight())
			{
				MsgRotateRight msgMove;
				msgMove.m_value = 4;
				SendServerMsg(msgMove, sizeof(msgMove));
			}
			if (controller->IsUp())
			{
				MsgRotateDown msgMove;
				msgMove.m_value = 4;
				SendServerMsg(msgMove, sizeof(msgMove));
			}
			if (controller->IsDown())
			{
				MsgRotateUp msgMove;
				msgMove.m_value = 4;
				SendServerMsg(msgMove, sizeof(msgMove));
			}
			if (controller->IsForward())
			{
				MsgMoveForward msgMove;
				msgMove.m_value = 16;
				SendServerMsg(msgMove, sizeof(msgMove));
			}
			if (controller->IsBackward())
			{
				MsgMoveBackward msgMove;
				msgMove.m_value = 16;
				SendServerMsg(msgMove, sizeof(msgMove));
			}
		}

		while(const char *buffer = RecvServerMsg())
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
				m_eyeColorArray[CommonParams::OBSERVER_EYE_SIZE - msgSendPhoton->m_posY - 1][msgSendPhoton->m_posX] = msgSendPhoton->m_color;
				m_eyeUpdateTimeArray[CommonParams::OBSERVER_EYE_SIZE - msgSendPhoton->m_posY - 1][msgSendPhoton->m_posX] = time;
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

	__int64 timeEllapsed;
	__int64 timeStart;
	__int64 timeDelta;

	QueryPerformanceFrequency((LARGE_INTEGER*)(&timeDelta));

	__int64 timeToWait = (double)timeDelta * (double)30 / 1000000.0f;

	QueryPerformanceCounter((LARGE_INTEGER*)(&timeStart));

	timeEllapsed = timeStart;

	while ((timeEllapsed - timeStart) < timeToWait)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)(&timeEllapsed));

	};
	//Sleep(1); // work imitation
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

void Observer::GetStateParams(VectorInt32Math &outPosition, uint16_t &outMovingProgress, int16_t &outLatitude,
	int16_t &outLongitude, VectorInt32Math &outEatenCrumbPos)
{
	std::lock_guard<std::mutex> guard(s_observerStateParamsMutex);
	outPosition = m_position;
	outMovingProgress = m_movingProgress;
	outLatitude = m_latitude;
	outLongitude = m_longitude;
	outEatenCrumbPos = m_eatenCrumbPos;
	m_eatenCrumbPos = VectorInt32Math::ZeroVector;
}

void Observer::GetStatisticsParams(uint32_t &outQuantumOfTimePerSecond, uint32_t &outUniverseThreadsNum,
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

const char* Observer::RecvServerMsg()
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

bool Observer::SendServerMsg(const MsgBase &msg, int32_t msgSize)
{
	struct sockaddr_in serverInfo;
	int len = sizeof(serverInfo);
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_port = htons(m_port);
	serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (sendto(m_socketC, (const char*)&msg, msgSize, 0, (sockaddr*)&serverInfo, len) == SOCKET_ERROR)
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



}
