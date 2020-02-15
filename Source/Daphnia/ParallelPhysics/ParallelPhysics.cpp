
#include "ParallelPhysics.h"
#include "vector"
#include "algorithm"
#include "array"
#include "thread"
#include "iostream"
#include "atomic"

namespace PPh
{

ParallelPhysics *s_parallelPhysicsInstance = nullptr;

std::vector< std::vector< std::vector<struct EtherCell> > > s_universe;
std::atomic<uint64_t> s_time = 0; // absolute universe time
std::atomic<int32_t> s_waitThreadsCount = 0; // thread synchronization variable
typedef std::vector<struct Photon> PhotonVector;
typedef std::shared_ptr<PhotonVector> SP_PhotonVector;

struct Photon
{
	VectorIntMath m_orientation;
	EtherColor m_color;
};

struct EtherCell
{
	int32_t m_type = EtherType::Space;
	EtherColor m_color;
	std::array<SP_PhotonVector, 8> m_photonsEven; 
	std::array<SP_PhotonVector, 8> m_photonsOdd;
};

void UniverseThread(int32_t x1, int32_t x2, bool *isSimulationRunning)
{
	while (*isSimulationRunning)
	{
		int isTimeOdd = s_time % 2;

		for (int32_t posX = x1; posX < x2; ++posX)
		{
			for (int32_t posY = 0; posY < ParallelPhysics::GetInstance()->GetUniverseSize().m_posY; ++posY)
			{
				for (int32_t posZ = 0; posZ < ParallelPhysics::GetInstance()->GetUniverseSize().m_posZ; ++posZ)
				{
				}
			}
		}
		--s_waitThreadsCount;
		while (s_time % 2 == isTimeOdd)
		{
		}
	}
	--s_waitThreadsCount;
}

bool ParallelPhysics::Init(const VectorIntMath &universeSize, uint8_t threadsCount)
{
	if (0 < universeSize.m_posX && 0 < universeSize.m_posY && 0 < universeSize.m_posZ)
	{
		s_universe.resize(universeSize.m_posX);
		for (auto &itY : s_universe)
		{
			itY.resize(universeSize.m_posY);
			for (auto &itZ : itY)
			{
				itZ.resize(universeSize.m_posZ);
			}
		}
		if (s_parallelPhysicsInstance)
		{
			delete s_parallelPhysicsInstance;
		}
		s_parallelPhysicsInstance = new ParallelPhysics();
		GetInstance()->m_universeSize = universeSize;
		GetInstance()->m_threadsCount = threadsCount;
		return true;
	}
	return false;
}

ParallelPhysics* ParallelPhysics::GetInstance()
{
	return s_parallelPhysicsInstance;
}

const VectorIntMath & ParallelPhysics::GetUniverseSize() const
{
	return m_universeSize;
}

std::thread s_simulationThread;
void ParallelPhysics::StartSimulation()
{
	s_simulationThread = std::thread([this]() {
		m_isSimulationRunning = true;
		int32_t lengthForThread = m_universeSize.m_posX / m_threadsCount;

		std::vector<std::thread> threads;
		threads.resize(m_threadsCount);

		s_waitThreadsCount = m_threadsCount + 1; // universe threads and observer thread
		std::thread observerThread = std::thread([this]()
		{
			while (m_isSimulationRunning)
			{
				int32_t isTimeOdd = s_time % 2;
				Observer::GetInstance()->PPhTick();
				--s_waitThreadsCount;
				while (s_time % 2 == isTimeOdd)
				{
				}
			}
			--s_waitThreadsCount;
		});

		int32_t lengthForThreadRemain = m_universeSize.m_posX - lengthForThread * m_threadsCount;
		int32_t beginX = 0;
		for (int ii = 0; ii < m_threadsCount; ++ii)
		{
			int32_t lengthX = lengthForThread;
			if (0 < lengthForThreadRemain)
			{
				++lengthX;
				--lengthForThreadRemain;
			}
			int32_t endX = beginX + lengthX;
			threads[ii] = std::thread(UniverseThread, beginX, endX, &m_isSimulationRunning);
			beginX = endX;
		}

		while (m_isSimulationRunning)
		{
			while (s_waitThreadsCount)
			{
			}
			s_waitThreadsCount = m_threadsCount + 1; // universe threads and observer thread
			++s_time;
		}
			/*observerThread.join();
			for (int ii = 0; ii < m_threadsCount; ++ii)
			{
				threads[ii].join();
			}*/
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

bool ParallelPhysics::InitEtherCell(const VectorIntMath &pos, EtherType::EEtherType type, const EtherColor &color)
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

bool ParallelPhysics::EmitPhoton(const VectorIntMath &pos, const VectorIntMath &orient)
{
	return true;
}

Observer* s_observer = nullptr;

void Observer::Init(const VectorIntMath &position, const SP_EyeState &eyeState)
{
	if (s_observer)
	{
		delete s_observer;
	}
	s_observer = new Observer();
	s_observer->m_position = position;
	s_observer->m_eyeState = eyeState;
	ParallelPhysics::GetInstance()->InitEtherCell(position, EtherType::Observer);
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
		for (int ii = 0; ii < eyeArray.size(); ++ii)
		{
			for (int jj = 0; jj < eyeArray[ii].size(); ++jj)
			{
				ParallelPhysics::GetInstance()->EmitPhoton(m_position, eyeArray[ii][jj]);
			}
		}
		std::cout << "emit photons" << std::endl;
	}
}

void Observer::ChangeOrientation(const SP_EyeState &eyeState)
{
	std::atomic_store(&m_newEyeState, eyeState);
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