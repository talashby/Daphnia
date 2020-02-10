
#include "ParallelPhysics.h"
#include "vector"

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

void Observer::Init(const VectorIntMath &position, const VectorIntMath &orientation)
{
	if (s_observer)
	{
		delete s_observer;
	}
	s_observer = new Observer();
	s_observer->m_position = position;
	s_observer->m_orientation = orientation;
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
		const uint32_t eyeNeuroneZoneLength = (EYE_FOV / EYE_SIZE) * 2;
		const uint32_t halfEyeNeuroneZoneLength = eyeNeuroneZoneLength / 2;
		for (int ii = 0; ii < EYE_SIZE; ++ii)
		{
			for (int jj = 0; jj < EYE_SIZE; ++jj)
			{
				int32_t shiftY = 0 - EYE_FOV + eyeNeuroneZoneLength * ii + halfEyeNeuroneZoneLength;
				int32_t shiftZ = 0 - EYE_FOV + eyeNeuroneZoneLength * jj + halfEyeNeuroneZoneLength;
				PPh::VectorIntMath orientation = PPh::OrientationRotation(m_orientation, shiftY, shiftZ);
				ParallelPhysics::GetInstance()->EmitPhoton(m_position, orientation);
			}
		}
	}
}

}