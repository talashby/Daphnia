#pragma once

#include "PPhHelpers.h"

namespace PPh
{

struct EtherType
{
	enum EEtherType
	{
		Space = 0,
		Crumb,
		Block,
		Observer
	};
};

class ParallelPhysics
{
public:

	static bool Init(const VectorIntMath &universeSize); // returns true if success
	static ParallelPhysics* GetInstance();

	const VectorIntMath & GetUniverseSize() const;

	void StartSimulation();
	void StopSimulation();

	bool InitEtherCell(const VectorIntMath &pos, EtherType::EEtherType type, const EtherColor &color = EtherColor()); // returns true if success
	bool EmitPhoton(const VectorIntMath &pos, const VectorIntMath &orient);

private:
	ParallelPhysics();

	VectorIntMath m_universeSize = VectorIntMath::ZeroVector;
};

class Observer
{
public:
	static void Init(const VectorIntMath &position, const VectorIntMath &orientation);

	static Observer* GetInstance();

	void PPhTick();

private:
	VectorIntMath m_position = VectorIntMath::ZeroVector;
	VectorIntMath m_orientation = VectorIntMath::ZeroVector;

	const int32_t EYE_SIZE = 32; // pixels
	const int32_t EYE_IMAGE_DELAY = 300; // quantum of time
	const uint32_t EYE_FOV = MAX_INT/2; // quantum of length (MAX_INT/2 - 90 degrees; MAX_INT - 180 degrees; 2*MAX_INT - 360 degrees)

	const int32_t ECHOLOCATION_FREQUENCY = 1000; // quantum of time
	int32_t m_echolocationCounter = 0;
};
}
