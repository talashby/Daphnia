#pragma once

#include "PPhHelpers.h"
#include "ServerProtocol.h"
#include "memory"
#include "array"
#include "vector"
#include "atomic"

namespace PPh
{

typedef int32_t PhotonParam; // warning! Depends on OBSERVER_EYE_SIZE
constexpr int32_t UPDATE_EYE_TEXTURE_OUT = 20; // milliseconds
constexpr int32_t STATISTIC_REQUEST_PERIOD = 900; // milliseconds
typedef std::array< std::array<OrientationVectorMath, CommonParams::OBSERVER_EYE_SIZE>, CommonParams::OBSERVER_EYE_SIZE> EyeArray;
typedef std::shared_ptr< EyeArray > SP_EyeState;
typedef std::array< std::array<EtherColor, CommonParams::OBSERVER_EYE_SIZE>, CommonParams::OBSERVER_EYE_SIZE> EyeColorArray;
typedef std::array< std::array<uint64_t, CommonParams::OBSERVER_EYE_SIZE>, CommonParams::OBSERVER_EYE_SIZE> EyeUpdateTimeArray;
typedef std::shared_ptr< EyeColorArray > SP_EyeColorArray;

class ObserverClient
{
public:
	static void Init(ObserverClient *observer = nullptr);
	static ObserverClient* Instance();
	ObserverClient() = default;
	virtual ~ObserverClient() = default;

	void StartSimulation();
	void StopSimulation();
	bool IsSimulationRunning() const;

	uint64_t GetLastObserverID() const; // returns last observer ID successfully connected to server

	void PPhTick();

	SP_EyeColorArray GrabTexture();
	VectorInt32Math GetPosition() const;

	const VectorInt32Math& GetOrientMinChanger() const;
	const VectorInt32Math& GetOrientMaxChanger() const;

	void GetStateExtParams(VectorInt32Math &outPosition, uint16_t &outMovingProgress, int16_t &outLatitude, int16_t &outLongitude, 
		bool &outIsEatenCrumb) const;
	VectorInt32Math GrabEatenCrumbPos();

	void GetStatisticsParams(uint32_t &outQuantumOfTimePerSecond, uint32_t &outUniverseThreadsNum,
		uint32_t &outTickTimeMusAverageUniverseThreadsMin, // average tick time in microseconds
		uint32_t &outTickTimeMusAverageUniverseThreadsMax, // average tick time in microseconds
		uint32_t &outTickTimeMusAverageObserverThread, // average tick time in microseconds
		uint64_t &outClientServerPerformanceRatio,
		uint64_t &outServerClientPerformanceRatio);

	// Set motor neurons
	void SetIsLeft(bool value) { m_isLeft = value; }
	void SetIsRight(bool value) { m_isRight = value; }
	void SetIsUp(bool value) { m_isUp = value; }
	void SetIsDown(bool value) { m_isDown = value; }
	void SetIsForward(bool value) { m_isForward = value; }
	void SetIsBackward(bool value) { m_isBackward = value; }

protected:
	const char* RecvServerMsg(); // returns nullptr if error occur
	bool SendServerMsg(const MsgBase &msg, int32_t msgSize); // returns false if error occur
	virtual void HandleReceivedMessage(const char *buffer);

private:
	bool m_isSimulationRunning = false;

	uint32_t m_socketC;
	uint32_t m_port;

	const int32_t EYE_IMAGE_DELAY = 5000; // quantum of time
	const int32_t ECHOLOCATION_FREQUENCY = 1; // quantum of time
	int32_t m_echolocationCounter = 0;

	EyeColorArray m_eyeColorArray = EyeColorArray(); // photon (x,y) placed to [CommonParams::OBSERVER_EYE_SIZE - y -1][x] for simple copy to texture purpose
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
	bool m_isEatenCrumb = false;
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

	// motor neurons
	bool m_isLeft=false, m_isRight=false, m_isUp=false, m_isDown=false, m_isForward=false, m_isBackward=false;
};

}
