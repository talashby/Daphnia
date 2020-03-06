#pragma once

#include "PPhHelpers.h"
#include "memory"
#include "array"
#include "vector"

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

	static bool Init(const VectorInt32Math &universeSize, uint8_t threadsCount); // returns true if success. threadsCount 0 means simulate near observer
	static ParallelPhysics* GetInstance();

	const VectorInt32Math & GetUniverseSize() const;

	void StartSimulation();
	void StopSimulation();
	bool IsSimulationRunning() const;

	bool InitEtherCell(const VectorInt32Math &pos, EtherType::EEtherType type, const EtherColor &color = EtherColor()); // returns true if success
	bool EmitPhoton(const VectorInt32Math &pos, const struct Photon &photon);

	static void SetNeedUpdateSimulationBoxes();

	static uint64_t GetFPS();
	static bool IsHighPrecisionStatsEnabled();
	static uint64_t GetTickTimeNsObserverThread(); // average tick time in nanoseconds
	static std::vector<uint64_t> GetTickTimeNsUniverseThreads(); // average tick time in nanoseconds

private:
	ParallelPhysics();

	static int32_t GetCellPhotonIndex(const VectorInt32Math &unitVector);
	bool IsPosInBounds(const VectorInt32Math &pos);
	void AdjustSimulationBoxes();
	void AdjustSizeByBounds(VectorInt32Math &size);

	VectorInt32Math m_universeSize = VectorInt32Math::ZeroVector;
	uint8_t m_threadsCount = 1;
	bool m_bSimulateNearObserver = false;
	bool m_isSimulationRunning = false;
};

typedef int16_t PhotonParam; // warning! Depends on OBSERVER_EYE_SIZE
constexpr int32_t OBSERVER_EYE_SIZE = 16; // pixels
constexpr int32_t UPDATE_EYE_TEXTURE_OUT = 20; // milliseconds
typedef std::array< std::array<OrientationVectorMath, OBSERVER_EYE_SIZE>, OBSERVER_EYE_SIZE> EyeArray;
typedef std::shared_ptr< EyeArray > SP_EyeState;
typedef std::array< std::array<EtherColor, OBSERVER_EYE_SIZE>, OBSERVER_EYE_SIZE> EyeColorArray;
typedef std::array< std::array<uint64_t, OBSERVER_EYE_SIZE>, OBSERVER_EYE_SIZE> EyeUpdateTimeArray;
typedef std::shared_ptr< EyeColorArray > SP_EyeColorArray;

class Observer
{
public:
	static void Init(const VectorInt32Math &position, const SP_EyeState &eyeState);

	static Observer* GetInstance();

	void PPhTick();

	void ChangeOrientation(const SP_EyeState &eyeState);
	SP_EyeColorArray GrabTexture();
	VectorInt32Math GetPosition() const;
	void SetNewPosition(const VectorInt32Math &pos);
	VectorInt32Math GetNewPosition() const;

	const VectorInt32Math& GetOrientMinChanger() const;
	const VectorInt32Math& GetOrientMaxChanger() const;
private:
	friend class ParallelPhysics;
	void SetPosition(const VectorInt32Math &pos);
	void CalculateOrientChangers(const EyeArray &eyeArray);
	// Math
	//static bool NormalizeHorizontal(VectorIntMath &orient); // returns false if vector is not orientation vector
// Calculate orientation shift by horizontal and vertical. Be sure shiftH < MAX_INT && shiftV < MAX_INT
	//static VectorIntMath OrientationShift(const VectorIntMath &orient, int32_t shiftH, int32_t shiftV);

	VectorInt32Math m_position = VectorInt32Math::ZeroVector;
	VectorInt32Math m_newPosition = VectorInt32Math::ZeroVector;
	SP_EyeState m_eyeState;
	SP_EyeState m_newEyeState; // Used from different threads

	const int32_t EYE_IMAGE_DELAY = 3000; // quantum of time
	//const uint32_t EYE_FOV = PPH_INT_MAX/2; // quantum of length (MAX_INT/2 - 90 degrees; MAX_INT - 180 degrees; 2*MAX_INT - 360 degrees)

	const int32_t ECHOLOCATION_FREQUENCY = 1; // quantum of time
	int32_t m_echolocationCounter = 0;

	EyeColorArray m_eyeColorArray = EyeColorArray();
	EyeUpdateTimeArray m_eyeUpdateTimeArray = EyeUpdateTimeArray();

	int64_t m_lastTextureUpdateTime = 0;
	SP_EyeColorArray m_spEyeColorArrayOut;

	VectorInt32Math m_orientMinChanger;
	VectorInt32Math m_orientMaxChanger;
};
}
