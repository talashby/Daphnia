
#include "ParallelPhysics.h"
#include "vector"
#include "algorithm"
#include "array"
#include "thread"
#include "iostream"
#include "atomic"
#include "chrono"

namespace PPh
{

ParallelPhysics *s_parallelPhysicsInstance = nullptr;

std::vector< std::vector< std::vector<struct EtherCell> > > s_universe;
std::atomic<uint64_t> s_time = 0; // absolute universe time
std::atomic<int32_t> s_waitThreadsCount = 0; // thread synchronization variable
std::vector<BoxIntMath> s_threadSimulateBounds; // [minVector; maxVector)

// stats
uint64_t s_quantumOfTimePerSecond = 0;
#define HIGH_PRECISION_STATS 1
std::vector<uint64_t> s_timingsUniverseThreads;
std::vector<uint64_t> s_TickTimeNsAverageUniverseThreads;
uint64_t s_timingsObserverThread;
uint64_t s_TickTimeNsAverageObserverThread;

struct Photon
{
	Photon()
	{}
	explicit Photon(const OrientationVectorMath &orientation) : m_orientation(orientation)
	{}
	EtherColor m_color;
	OrientationVectorMath m_orientation;
	PhotonParam m_param;
};

struct EtherCell
{
	int32_t m_type = EtherType::Space;
	EtherColor m_color;
	std::array <std::array<Photon, 26>, 2> m_photons;
};

bool ParallelPhysics::Init(const VectorInt32Math &universeSize, uint8_t threadsCount)
{
	if (0 < universeSize.m_posX && 0 < universeSize.m_posY && 0 < universeSize.m_posZ)
	{
		OrientationVectorMath::InitRandom();
		s_universe.resize(universeSize.m_posX);
		for (auto &itY : s_universe)
		{
			itY.resize(universeSize.m_posY);
			for (auto &itZ : itY)
			{
				EtherCell cell; // color is zero by default
				itZ.resize(universeSize.m_posZ, cell);
			}
		}
		if (s_parallelPhysicsInstance)
		{
			delete s_parallelPhysicsInstance;
		}
		s_parallelPhysicsInstance = new ParallelPhysics();
		GetInstance()->m_universeSize = universeSize;
		if (0 == threadsCount)
		{
			GetInstance()->m_bSimulateNearObserver = true;
			GetInstance()->m_threadsCount = 4;
		}
		else
		{
			GetInstance()->m_threadsCount = threadsCount;
		}
#ifdef HIGH_PRECISION_STATS
		s_timingsUniverseThreads.resize(GetInstance()->m_threadsCount);
		s_TickTimeNsAverageUniverseThreads.resize(GetInstance()->m_threadsCount);
#endif
		// fill bounds

		s_threadSimulateBounds.resize(GetInstance()->m_threadsCount);
		int32_t lengthForThread = GetInstance()->m_universeSize.m_posX / GetInstance()->m_threadsCount;
		int32_t lengthForThreadRemain = GetInstance()->m_universeSize.m_posX - lengthForThread * GetInstance()->m_threadsCount;
		int32_t beginX = 0;
		for (int ii = 0; ii < GetInstance()->m_threadsCount; ++ii)
		{
			int32_t lengthX = lengthForThread;
			if (0 < lengthForThreadRemain)
			{
				++lengthX;
				--lengthForThreadRemain;
			}
			int32_t endX = beginX + lengthX;
			s_threadSimulateBounds[ii].m_minVector = VectorInt32Math(beginX, 0, 0);
			s_threadSimulateBounds[ii].m_maxVector = VectorInt32Math(endX, GetInstance()->m_universeSize.m_posY, GetInstance()->m_universeSize.m_posZ);
			beginX = endX;
		}

		return true;
	}
	return false;
}


void UniverseThread(int32_t threadNum, bool *isSimulationRunning)
{
	while (*isSimulationRunning)
	{
#ifdef HIGH_PRECISION_STATS
		auto beginTime = std::chrono::high_resolution_clock::now();
#endif
		int isTimeOdd = s_time % 2;

		for (int32_t posX = s_threadSimulateBounds[threadNum].m_minVector.m_posX; posX < s_threadSimulateBounds[threadNum].m_maxVector.m_posX; ++posX)
		{
			for (int32_t posY = s_threadSimulateBounds[threadNum].m_minVector.m_posY; posY < s_threadSimulateBounds[threadNum].m_maxVector.m_posY; ++posY)
			{
				for (int32_t posZ = s_threadSimulateBounds[threadNum].m_minVector.m_posZ; posZ < s_threadSimulateBounds[threadNum].m_maxVector.m_posZ; ++posZ)
				{
					EtherCell &cell = s_universe[posX][posY][posZ];
					if (cell.m_type == EtherType::Observer)
					{
						continue;
					}
					for (int ii = 0; ii < cell.m_photons[isTimeOdd].size(); ++ii)
					{
						Photon &photon = cell.m_photons[isTimeOdd][ii];
						if (photon.m_color.m_colorA != 0)
						{
							if (cell.m_type == EtherType::Crumb || cell.m_type == EtherType::Block)
							{
								photon.m_orientation *= -1;
								uint8_t tmpA = photon.m_color.m_colorA;
								photon.m_color = cell.m_color;
								photon.m_color.m_colorA = tmpA;
							}
    						if (photon.m_color.m_colorA > 15)
							{
								photon.m_color.m_colorA -= 15;
								bool result = ParallelPhysics::GetInstance()->EmitPhoton({ posX, posY, posZ }, photon);
								if (result)
								{
									photon.m_color = EtherColor::ZeroColor;
								}
							}
							else
							{
								photon.m_color = EtherColor::ZeroColor;
							}
						}
					}
				}
			}
		}
#ifdef HIGH_PRECISION_STATS
		auto endTime = std::chrono::high_resolution_clock::now();
		auto dif = endTime - beginTime;
		s_timingsUniverseThreads[threadNum] += std::chrono::duration_cast<std::chrono::nanoseconds>(dif).count();
#endif
		--s_waitThreadsCount;
		while (s_time % 2 == isTimeOdd)
		{
		}
	}
	--s_waitThreadsCount;
}

ParallelPhysics* ParallelPhysics::GetInstance()
{
	return s_parallelPhysicsInstance;
}

const VectorInt32Math & ParallelPhysics::GetUniverseSize() const
{
	return m_universeSize;
}

void ParallelPhysics::AdjustSimulationBoxes()
{
	VectorInt32Math observerPos = Observer::GetInstance()->GetPosition();
	VectorInt32Math boundSize(12, 12, 12);
	VectorInt32Math boundsMax = observerPos + boundSize;
	boundSize *= -1;
	VectorInt32Math boundsMin = observerPos + boundSize;
	AdjustSizeByBounds(boundsMax);
	AdjustSizeByBounds(boundsMin);
	s_threadSimulateBounds[0] = BoxIntMath(boundsMin, { boundsMax.m_posX, observerPos.m_posY, observerPos.m_posZ });
	s_threadSimulateBounds[1] = BoxIntMath({ boundsMin.m_posX, observerPos.m_posY, boundsMin.m_posZ },
		{ boundsMax.m_posX, boundsMax.m_posY, observerPos.m_posZ });
	s_threadSimulateBounds[2] = BoxIntMath({ boundsMin.m_posX, observerPos.m_posY, observerPos.m_posZ }, boundsMax);
	s_threadSimulateBounds[3] = BoxIntMath({ boundsMin.m_posX, boundsMin.m_posY, observerPos.m_posZ },
		{ boundsMax.m_posX, observerPos.m_posY, boundsMax.m_posZ });
}

std::thread s_simulationThread;
void ParallelPhysics::StartSimulation()
{
	s_simulationThread = std::thread([this]() {
		m_isSimulationRunning = true;
		std::vector<std::thread> threads;
		threads.resize(m_threadsCount);

		s_waitThreadsCount = m_threadsCount + 1; // universe threads and observer thread
		std::thread observerThread = std::thread([this]()
		{
			while (m_isSimulationRunning)
			{
#ifdef HIGH_PRECISION_STATS
				auto beginTime = std::chrono::high_resolution_clock::now();
#endif
				int32_t isTimeOdd = s_time % 2;
				Observer::GetInstance()->PPhTick();
#ifdef HIGH_PRECISION_STATS
				auto endTime = std::chrono::high_resolution_clock::now();
				s_timingsObserverThread += std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - beginTime).count();
#endif
				--s_waitThreadsCount;
				while (s_time % 2 == isTimeOdd)
				{
				}
			}
			--s_waitThreadsCount;
		});

		if (m_bSimulateNearObserver)
		{
			AdjustSimulationBoxes();
		}
		for (int ii = 0; ii < m_threadsCount; ++ii)
		{
			threads[ii] = std::thread(UniverseThread, ii, &m_isSimulationRunning);
		}

		int64_t lastTime = GetTimeMs();
		uint64_t lastTimeUniverse = 0;
		while (m_isSimulationRunning)
		{
			while (s_waitThreadsCount)
			{
			}
			s_waitThreadsCount = m_threadsCount + 1; // universe threads and observer thread
			if (Observer::GetInstance()->GetPosition() != Observer::GetInstance()->GetNewPosition())
			{
				GetInstance()->InitEtherCell(Observer::GetInstance()->GetNewPosition(), EtherType::Observer);
				GetInstance()->InitEtherCell(Observer::GetInstance()->GetPosition(), EtherType::Space);
				Observer::GetInstance()->SetPosition(Observer::GetInstance()->GetNewPosition());
				if (m_bSimulateNearObserver)
				{
					AdjustSimulationBoxes();
				}
			}
			if (GetTimeMs() - lastTime >= 1000)
			{
				s_quantumOfTimePerSecond = s_time - lastTimeUniverse;
#ifdef HIGH_PRECISION_STATS
				for (int ii = 0; ii < s_timingsUniverseThreads.size(); ++ii)
				{
					if (s_timingsUniverseThreads[ii] > 0)
					{
						s_TickTimeNsAverageUniverseThreads[ii] = s_timingsUniverseThreads[ii] / s_quantumOfTimePerSecond;
						s_timingsUniverseThreads[ii] = 0;
					}
				}
				if (s_timingsObserverThread > 0)
				{
					s_TickTimeNsAverageObserverThread = s_timingsObserverThread / s_quantumOfTimePerSecond;
					s_timingsObserverThread = 0;
				}
#endif
				lastTime = GetTimeMs();
				lastTimeUniverse = s_time;
			}
			++s_time;
		}
		observerThread.join();
		for (int ii = 0; ii < m_threadsCount; ++ii)
		{
			threads[ii].join();
		}
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

bool ParallelPhysics::IsPosInBounds(const VectorInt32Math &pos)
{
	const VectorInt32Math &size = GetUniverseSize();
	if (pos.m_posX < 0 || pos.m_posY < 0 || pos.m_posZ < 0)
	{
		return false;
	}
	if (pos.m_posX >= size.m_posX || pos.m_posY >= size.m_posY || pos.m_posZ >= size.m_posZ)
	{
		return false;
	}
	return true;
}

void ParallelPhysics::AdjustSizeByBounds(VectorInt32Math &size)
{
	const VectorInt32Math &universeSize = GetUniverseSize();
	size.m_posX = std::max(0, size.m_posX);
	size.m_posY = std::max(0, size.m_posY);
	size.m_posZ = std::max(0, size.m_posZ);

	size.m_posX = std::min(universeSize.m_posX, size.m_posX);
	size.m_posY = std::min(universeSize.m_posY, size.m_posY);
	size.m_posZ = std::min(universeSize.m_posZ, size.m_posZ);
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
				return true;
			}
		}
	}
	return false;
}

bool ParallelPhysics::EmitPhoton(const VectorInt32Math &pos, const Photon &photon)
{
	const OrientationVectorMath &orient = photon.m_orientation;
	VectorInt32Math unitVector = VectorInt32Math::ZeroVector;
	if (std::abs(orient.m_posX) > OrientationVectorMath::GetRandomNumber())
	{
		unitVector.m_posX = OrientationVectorMath::Sign(orient.m_posX);
	}
	if (std::abs(orient.m_posY) > OrientationVectorMath::GetRandomNumber())
	{
		unitVector.m_posY = OrientationVectorMath::Sign(orient.m_posY);
	}
	if (std::abs(orient.m_posZ) > OrientationVectorMath::GetRandomNumber())
	{
		unitVector.m_posZ = OrientationVectorMath::Sign(orient.m_posZ);
	}

	VectorInt32Math nextPos = pos + unitVector;
	if (IsPosInBounds(nextPos))
	{
		int isTimeOdd = (s_time + 1) % 2; // will be handle on next quantum of time

		EtherCell &cell = s_universe[nextPos.m_posX][nextPos.m_posY][nextPos.m_posZ];
		int32_t cellPhotonIndex = GetCellPhotonIndex(unitVector);
		Photon &photonCell = cell.m_photons[isTimeOdd][cellPhotonIndex];
		if (photonCell.m_color.m_colorA > 0)
		{
			return false;
		}
		photonCell = photon;
	}
	
	return true;
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

void Observer::Init(const VectorInt32Math &position, const SP_EyeState &eyeState)
{
	if (s_observer)
	{
		delete s_observer;
	}
	s_observer = new Observer();
	s_observer->m_position = position;
	s_observer->m_newPosition = position;
	s_observer->m_eyeState = eyeState;
	ParallelPhysics::GetInstance()->InitEtherCell(position, EtherType::Observer);
	s_observer->m_lastTextureUpdateTime = GetTimeMs();
}

PPh::Observer* Observer::GetInstance()
{
	return s_observer;
}

void Observer::PPhTick()
{
	--m_echolocationCounter;
	if (m_echolocationCounter <= 0)
	{ // emit photons
		m_echolocationCounter = ECHOLOCATION_FREQUENCY;

		SP_EyeState newEyeState;
		std::atomic_store(&newEyeState, m_newEyeState);
		if (m_newEyeState && m_eyeState != m_newEyeState)
		{
			m_eyeState = m_newEyeState;
		}
		const EyeArray &eyeArray = *m_eyeState;
		{
			int32_t ii = OrientationVectorMath::GetRandomNumber() % (OBSERVER_EYE_SIZE / 2);
			int32_t jj = OrientationVectorMath::GetRandomNumber() % (OBSERVER_EYE_SIZE / 2);
			Photon photon(eyeArray[ii][jj]);
			photon.m_param = ii + jj * OBSERVER_EYE_SIZE;
			photon.m_color.m_colorA = 255;
			ParallelPhysics::GetInstance()->EmitPhoton(m_position, photon);
		}
		{
			int32_t ii = OrientationVectorMath::GetRandomNumber() % (OBSERVER_EYE_SIZE / 2) + (OBSERVER_EYE_SIZE / 2);
			int32_t jj = OrientationVectorMath::GetRandomNumber() % (OBSERVER_EYE_SIZE / 2);
			Photon photon(eyeArray[ii][jj]);
			photon.m_param = ii + jj * OBSERVER_EYE_SIZE;
			photon.m_color.m_colorA = 255;
			ParallelPhysics::GetInstance()->EmitPhoton(m_position, photon);
		}
		{
			int32_t ii = OrientationVectorMath::GetRandomNumber() % (OBSERVER_EYE_SIZE / 2);
			int32_t jj = OrientationVectorMath::GetRandomNumber() % (OBSERVER_EYE_SIZE / 2) + (OBSERVER_EYE_SIZE / 2);
			Photon photon(eyeArray[ii][jj]);
			photon.m_param = ii + jj * OBSERVER_EYE_SIZE;
			photon.m_color.m_colorA = 255;
			ParallelPhysics::GetInstance()->EmitPhoton(m_position, photon);
		}
		{
			int32_t ii = OrientationVectorMath::GetRandomNumber() % (OBSERVER_EYE_SIZE / 2) + (OBSERVER_EYE_SIZE / 2);
			int32_t jj = OrientationVectorMath::GetRandomNumber() % (OBSERVER_EYE_SIZE / 2) + (OBSERVER_EYE_SIZE / 2);
			Photon photon(eyeArray[ii][jj]);
			photon.m_param = ii + jj * OBSERVER_EYE_SIZE;
			photon.m_color.m_colorA = 255;
			ParallelPhysics::GetInstance()->EmitPhoton(m_position, photon);
		}
	}

	// receive photons back
	int isTimeOdd = s_time % 2;
	EtherCell &cell = s_universe[m_position.m_posX][m_position.m_posY][m_position.m_posZ];
	for (int ii = 0; ii < cell.m_photons[isTimeOdd].size(); ++ii)
	{
		Photon &photon = cell.m_photons[isTimeOdd][ii];
		if (photon.m_color.m_colorA > 0)
		{
			int32_t yPos = photon.m_param / OBSERVER_EYE_SIZE;
			int32_t xPos = photon.m_param - yPos * OBSERVER_EYE_SIZE;
			m_eyeColorArray[xPos][yPos] = photon.m_color;
			m_eyeUpdateTimeArray[xPos][yPos] = s_time;
			photon.m_color = EtherColor::ZeroColor;
		}
	}

	// update eye texture
	if (GetTimeMs() - m_lastTextureUpdateTime > UPDATE_EYE_TEXTURE_OUT)
	{
		m_lastTextureUpdateTime = GetTimeMs();
		SP_EyeColorArray spEyeColorArrayOut;
		std::atomic_store(&spEyeColorArrayOut, m_spEyeColorArrayOut);
		if (!spEyeColorArrayOut)
		{
			spEyeColorArrayOut = std::make_shared<EyeColorArray>();
			EyeColorArray &eyeColorArray = *spEyeColorArrayOut;
			for (int ii = 0; ii < eyeColorArray.size(); ++ii)
			{
				for (int jj = 0; jj < eyeColorArray[ii].size(); ++jj)
				{
					eyeColorArray[ii][jj] = m_eyeColorArray[ii][jj];
					int64 timeDiff = s_time - m_eyeUpdateTimeArray[ii][jj];
					uint8_t alpha = m_eyeColorArray[ii][jj].m_colorA;
					if (timeDiff < EYE_IMAGE_DELAY)
					{
						alpha = alpha * (EYE_IMAGE_DELAY - timeDiff) / EYE_IMAGE_DELAY;
					}
					else
					{
						alpha = 0;
					}
					eyeColorArray[ii][jj].m_colorA = alpha;
				}
			}
			std::atomic_store(&m_spEyeColorArrayOut, spEyeColorArrayOut);
		}
	}
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

void Observer::SetNewPosition(const VectorInt32Math &pos)
{
	m_newPosition = pos;
}

PPh::VectorInt32Math Observer::GetNewPosition() const
{
	return m_newPosition;
}

void Observer::SetPosition(const VectorInt32Math &pos)
{
	m_position = pos;
}

/*
bool Observer::NormalizeHorizontal(VectorIntMath &orient)
{
	// check if exists bigger than max values
	uint32_t absXY = std::max(std::abs(orient.m_posX), std::abs(orient.m_posY));
	absXY = std::max((uint32_t)PPH_INT_MAX, absXY);
	int32_t dif = absXY - PPH_INT_MAX;

	if (0 < dif)
	{
		if (orient.m_posX == PPH_INT_MAX)
		{
			orient.m_posX -= dif;
		}
		else if (orient.m_posX == PPH_INT_MIN)
		{
			orient.m_posX += dif;
		}
		else if (orient.m_posY == PPH_INT_MAX)
		{
			orient.m_posY -= dif;
		}
		else if (orient.m_posY == PPH_INT_MIN)
		{
			orient.m_posY += dif;
		}
		else
		{
			return false; // error
		}

		orient.m_posX = std::min(orient.m_posX, PPH_INT_MAX);
		orient.m_posX = std::max(orient.m_posX, PPH_INT_MIN);
		orient.m_posY = std::min(orient.m_posY, PPH_INT_MAX);
		orient.m_posY = std::max(orient.m_posY, PPH_INT_MIN);
	}

	return true;
}

VectorIntMath Observer::OrientationShift(const VectorIntMath &orient, int32_t shiftH, int32_t shiftV)
{
	VectorIntMath result = orient;

	if (PPH_INT_MAX == orient.m_posX)
	{
		result.m_posY += shiftH;
	}
	else if (PPH_INT_MIN == orient.m_posX)
	{
		result.m_posY -= shiftH;
	}
	else if (PPH_INT_MAX == orient.m_posY)
	{
		result.m_posX -= shiftH;
	}
	else if (PPH_INT_MIN == orient.m_posY)
	{
		result.m_posX += shiftH;
	}
	else
	{
		return VectorIntMath::ZeroVector; // error
	}

	NormalizeHorizontal(result);

	return result;
}*/

}