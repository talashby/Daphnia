#pragma once

#include "PPhHelpers.h"
#include "ServerProtocol.h"
#include "memory"
#include "array"
#include "vector"
#include "atomic"

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

	static bool Init(const VectorInt32Math &universeSize); // returns true if success. threadsCount 0 means simulate near observer
	static bool SaveUniverse(const std::string &fileName);
	static ParallelPhysics* GetInstance();

	const VectorInt32Math &GetUniverseSize() const;

	void StartSimulation();
	void StopSimulation();
	bool IsSimulationRunning() const;

	bool InitEtherCell(const VectorInt32Math &pos, EtherType::EEtherType type, const EtherColor &color = EtherColor()); // returns true if success

	bool GetNextCrumb(VectorInt32Math &outCrumbPos, EtherColor &outCrumbColor);
	static void EtherCellSetCrumbActor(const VectorInt32Math &pos, AActor *crumbActor);
	static AActor* EtherCellGetCrumbActor(const VectorInt32Math &pos);
private:
	ParallelPhysics();

	VectorInt32Math m_universeSize = VectorInt32Math::ZeroVector;
	bool m_isSimulationRunning = false;
};

typedef int32_t PhotonParam; // warning! Depends on OBSERVER_EYE_SIZE
constexpr int32_t UPDATE_EYE_TEXTURE_OUT = 20; // milliseconds
constexpr int32_t STATISTIC_REQUEST_PERIOD = 900; // milliseconds
typedef std::array< std::array<OrientationVectorMath, CommonParams::OBSERVER_EYE_SIZE>, CommonParams::OBSERVER_EYE_SIZE> EyeArray;
typedef std::shared_ptr< EyeArray > SP_EyeState;
typedef std::array< std::array<EtherColor, CommonParams::OBSERVER_EYE_SIZE>, CommonParams::OBSERVER_EYE_SIZE> EyeColorArray;
typedef std::array< std::array<uint64_t, CommonParams::OBSERVER_EYE_SIZE>, CommonParams::OBSERVER_EYE_SIZE> EyeUpdateTimeArray;
typedef std::shared_ptr< EyeColorArray > SP_EyeColorArray;

class Observer
{
public:
	static void Init();

	static Observer* GetInstance();

	void PPhTick(uint64_t socketC, uint32_t port);

	void ChangeOrientation(const SP_EyeState &eyeState);
	SP_EyeColorArray GrabTexture();
	VectorInt32Math GetPosition() const;

	const VectorInt32Math& GetOrientMinChanger() const;
	const VectorInt32Math& GetOrientMaxChanger() const;

	void GetStateParams(VectorInt32Math &outPosition, uint16_t &outMovingProgress, int16_t &outLatitude, int16_t &outLongitude, 
		VectorInt32Math &outEatenCrumbPos);

	void GetStatisticsParams(uint32_t &outQuantumOfTimePerSecond, uint32_t &outUniverseThreadsNum,
		uint32_t &outTickTimeMusAverageUniverseThreadsMin, // average tick time in microseconds
		uint32_t &outTickTimeMusAverageUniverseThreadsMax, // average tick time in microseconds
		uint32_t &outTickTimeMusAverageObserverThread, // average tick time in microseconds
		uint64_t &outClientServerPerformanceRatio,
		uint64_t &outServerClientPerformanceRatio);

private:
	friend class ParallelPhysics;
	void SetPosition(const VectorInt32Math &pos);
	void CalculateOrientChangers(const EyeArray &eyeArray);
	OrientationVectorMath MaximizePPhOrientation(const VectorFloatMath &orientationVector);

	SP_EyeState m_eyeState;
	SP_EyeState m_newEyeState; // Used from different threads

	const int32_t EYE_IMAGE_DELAY = 5000; // quantum of time
	//const uint32_t EYE_FOV = PPH_INT_MAX/2; // quantum of length (MAX_INT/2 - 90 degrees; MAX_INT - 180 degrees; 2*MAX_INT - 360 degrees)

	const int32_t ECHOLOCATION_FREQUENCY = 1; // quantum of time
	int32_t m_echolocationCounter = 0;

	EyeColorArray m_eyeColorArray = EyeColorArray();
	EyeUpdateTimeArray m_eyeUpdateTimeArray = EyeUpdateTimeArray();

	int64_t m_lastTextureUpdateTime = 0;
	SP_EyeColorArray m_spEyeColorArrayOut;

	VectorInt32Math m_orientMinChanger;
	VectorInt32Math m_orientMaxChanger;

	int16_t m_latitude = 0;
	int16_t m_longitude = 0;
	VectorInt32Math m_position = VectorInt32Math::ZeroVector;
	uint16_t m_movingProgress = 0;
	uint32_t m_eatenCrumbNum = 0;
	VectorInt32Math m_eatenCrumbPos = VectorInt32Math::ZeroVector;

	int64_t m_lastUpdateStateExtTime = 0;
	int64_t m_lastStatisticRequestTime = 0;

	// statistics
	uint32_t m_quantumOfTimePerSecond = 0;
	uint32_t m_universeThreadsNum = 0;
	uint32_t m_TickTimeMusAverageUniverseThreadsMin = 0;
	uint32_t m_TickTimeMusAverageUniverseThreadsMax = 0;
	uint32_t m_TickTimeMusAverageObserverThread = 0;
	uint64_t m_clientServerPerformanceRatio = 0;
	uint64_t m_serverClientPerformanceRatio = 0;
};

}
