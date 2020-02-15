#pragma once

#include "PPhHelpers.h"
#include "memory"

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

	static bool Init(const VectorIntMath &universeSize, uint8_t threadsCount); // returns true if success
	static ParallelPhysics* GetInstance();

	const VectorIntMath & GetUniverseSize() const;

	void StartSimulation();
	void StopSimulation();
	bool IsSimulationRunning() const;

	bool InitEtherCell(const VectorIntMath &pos, EtherType::EEtherType type, const EtherColor &color = EtherColor()); // returns true if success
	bool EmitPhoton(const VectorIntMath &pos, const VectorIntMath &orient);

private:
	ParallelPhysics();

	VectorIntMath m_universeSize = VectorIntMath::ZeroVector;
	uint8_t m_threadsCount = 1;
	bool m_isSimulationRunning = false;
};

constexpr int32_t OBSERVER_EYE_SIZE = 32; // pixels
typedef std::array< std::array<VectorIntMath, OBSERVER_EYE_SIZE>, OBSERVER_EYE_SIZE> EyeArray;
typedef std::shared_ptr< EyeArray > SP_EyeState;

class Observer
{
public:
	static void Init(const VectorIntMath &position, const SP_EyeState &eyeState);

	static Observer* GetInstance();

	void PPhTick();

	void ChangeOrientation(const SP_EyeState &eyeState);
private:

	// Math
	//static bool NormalizeHorizontal(VectorIntMath &orient); // returns false if vector is not orientation vector
// Calculate orientation shift by horizontal and vertical. Be sure shiftH < MAX_INT && shiftV < MAX_INT
	//static VectorIntMath OrientationShift(const VectorIntMath &orient, int32_t shiftH, int32_t shiftV);

	VectorIntMath m_position = VectorIntMath::ZeroVector;
	SP_EyeState m_eyeState;
	SP_EyeState m_newEyeState; // Used from different threads

	const int32_t EYE_IMAGE_DELAY = 300; // quantum of time
	//const uint32_t EYE_FOV = PPH_INT_MAX/2; // quantum of length (MAX_INT/2 - 90 degrees; MAX_INT - 180 degrees; 2*MAX_INT - 360 degrees)

	const int32_t ECHOLOCATION_FREQUENCY = 1000; // quantum of time
	int32_t m_echolocationCounter = 0;
};
}
