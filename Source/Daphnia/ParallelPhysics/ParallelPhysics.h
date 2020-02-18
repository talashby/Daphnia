#pragma once

#include "PPhHelpers.h"
#include "memory"
#include "array"

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
	bool EmitPhoton(const VectorIntMath &pos, const struct Photon &photon);

private:
	ParallelPhysics();

	static int32_t GetCellPhotonIndex(const VectorIntMath &unitVector);
	bool IsPosBounds(const VectorIntMath &pos);

	VectorIntMath m_universeSize = VectorIntMath::ZeroVector;
	uint8_t m_threadsCount = 1;
	bool m_isSimulationRunning = false;
};

constexpr int32_t OBSERVER_EYE_SIZE = 32; // pixels
constexpr int32_t UPDATE_EYE_TEXTURE_OUT = 200; // milliseconds
typedef std::array< std::array<VectorIntMath, OBSERVER_EYE_SIZE>, OBSERVER_EYE_SIZE> EyeArray;
typedef std::shared_ptr< EyeArray > SP_EyeState;
typedef std::array< std::array<EtherColor, OBSERVER_EYE_SIZE>, OBSERVER_EYE_SIZE> EyeColorArray;
typedef std::array< std::array<uint64_t, OBSERVER_EYE_SIZE>, OBSERVER_EYE_SIZE> EyeUpdateTimeArray;
typedef std::shared_ptr< EyeColorArray > SP_EyeColorArray;

class Observer
{
public:
	static void Init(const VectorIntMath &position, const SP_EyeState &eyeState);

	static Observer* GetInstance();

	void PPhTick();

	void ChangeOrientation(const SP_EyeState &eyeState);
	SP_EyeColorArray GrabTexture();
private:

	// Math
	//static bool NormalizeHorizontal(VectorIntMath &orient); // returns false if vector is not orientation vector
// Calculate orientation shift by horizontal and vertical. Be sure shiftH < MAX_INT && shiftV < MAX_INT
	//static VectorIntMath OrientationShift(const VectorIntMath &orient, int32_t shiftH, int32_t shiftV);

	VectorIntMath m_position = VectorIntMath::ZeroVector;
	SP_EyeState m_eyeState;
	SP_EyeState m_newEyeState; // Used from different threads

	const int32_t EYE_IMAGE_DELAY = 10000; // quantum of time
	//const uint32_t EYE_FOV = PPH_INT_MAX/2; // quantum of length (MAX_INT/2 - 90 degrees; MAX_INT - 180 degrees; 2*MAX_INT - 360 degrees)

	const int32_t ECHOLOCATION_FREQUENCY = 100; // quantum of time
	int32_t m_echolocationCounter = 0;

	EyeColorArray m_eyeColorArray = EyeColorArray();
	EyeUpdateTimeArray m_eyeUpdateTimeArray = EyeUpdateTimeArray();

	int64_t m_lastTextureUpdateTime = 0;
	SP_EyeColorArray m_spEyeColorArrayOut;
};
}
