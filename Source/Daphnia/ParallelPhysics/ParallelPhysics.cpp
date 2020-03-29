
#include "ParallelPhysics.h"
#include "vector"
#include "algorithm"
#include "array"
#include "thread"
#include "fstream"
#include "atomic"
#include "chrono"
#include "ServerProtocol.h"
#include "../MyPlayerController.h"

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#undef TEXT
#include <windows.h>
#include <winsock2.h>
#include "iosfwd"
#undef min
#undef max

namespace PPh
{

ParallelPhysics *s_parallelPhysicsInstance = nullptr;

std::vector< std::vector< std::vector<struct EtherCell> > > s_universe;
std::atomic<int32_t> s_waitThreadsCount = 0; // thread synchronization variable
// stats
uint64_t s_quantumOfTimePerSecond = 0;
#define HIGH_PRECISION_STATS 1
std::vector<uint64_t> s_timingsUniverseThreads;
std::vector<uint64_t> s_TickTimeNsAverageUniverseThreads;
uint64_t s_timingsObserverThread;
uint64_t s_TickTimeNsAverageObserverThread;

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

bool ParallelPhysics::Init(const VectorInt32Math &universeSize, uint8_t threadsCount)
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

	s_simulationThread = std::thread([this]() {
		m_isSimulationRunning = true;
		s_waitThreadsCount = 1; // observer thread
		std::thread observerThread = std::thread([this]()
		{
			while (m_isSimulationRunning)
			{
				Observer::GetInstance()->PPhTick();
				--s_waitThreadsCount;
			}
			--s_waitThreadsCount;
		});

		observerThread.join();
	});
}

void ParallelPhysics::StopSimulation()
{
	m_isSimulationRunning = false;
	s_simulationThread.join();
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

uint64_t ParallelPhysics::GetFPS()
{
	return s_quantumOfTimePerSecond;
}

bool ParallelPhysics::IsHighPrecisionStatsEnabled()
{
#ifdef HIGH_PRECISION_STATS
	return true;
#else
	return false;
#endif
}

uint64_t ParallelPhysics::GetTickTimeNsObserverThread()
{
	return s_TickTimeNsAverageObserverThread;
}

std::vector<uint64_t> ParallelPhysics::GetTickTimeNsUniverseThreads()
{
	return s_TickTimeNsAverageUniverseThreads;
}

Observer* s_observer = nullptr;

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

void Observer::PPhTick()
{

	struct sockaddr_in serverInfo;
	int len = sizeof(serverInfo);
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_port = htons(1234);
	serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
	static SOCKET socketC = 0;
	if (!socketC)
	{
		socketC = socket(AF_INET, SOCK_DGRAM, 0);
		u_long mode = 1;  // 1 to enable non-blocking socket
		ioctlsocket(socketC, FIONBIO, &mode);
	}
	else
	{
		MsgGetState msg;
		if (sendto(socketC, (const char*)&msg, sizeof(msg), 0, (sockaddr*)&serverInfo, len) != SOCKET_ERROR)
		{
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

			char buffer[MAX_PROTOCOL_BUFFER_SIZE];
			while(recvfrom(socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, &len) > 0)
			{
				MsgSendState *msgSendState = QueryMessage<MsgSendState>(buffer);
				static uint64_t time = 0;
				if (msgSendState)
				{
					time = msgSendState->m_time;
					Observer::GetInstance()->m_latitude = msgSendState->m_latitude;
					Observer::GetInstance()->m_longitude = msgSendState->m_longitude;
				}
				MsgSendPhoton *msgSendPhoton = QueryMessage<MsgSendPhoton>(buffer);
				if (msgSendPhoton)
				{
					// receive photons back // revert Y-coordinate because of texture format
					m_eyeColorArray[OBSERVER_EYE_SIZE - msgSendPhoton->m_posY - 1][msgSendPhoton->m_posX] = msgSendPhoton->m_color;
					m_eyeUpdateTimeArray[OBSERVER_EYE_SIZE - msgSendPhoton->m_posY - 1][msgSendPhoton->m_posX] = time;
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