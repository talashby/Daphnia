
#include "ParallelPhysics.h"
#include "vector"
#include "algorithm"
#include "array"

namespace PPh
{

struct EtherCell
{
	int32_t m_type = EtherType::Space;
	EtherColor m_color;
};

std::vector< std::vector< std::vector<EtherCell> > > s_universe;
ParallelPhysics *s_parallelPhysicsInstance = nullptr;

bool ParallelPhysics::Init(const VectorIntMath &universeSize)
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
		return true;
	}
	return false;
}

ParallelPhysics* ParallelPhysics::GetInstance()
{
	return s_parallelPhysicsInstance;
}

const PPh::VectorIntMath & ParallelPhysics::GetUniverseSize() const
{
	return m_universeSize;
}

void ParallelPhysics::StartSimulation()
{
	Observer::GetInstance()->PPhTick();
}

void ParallelPhysics::StopSimulation()
{

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