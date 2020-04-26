#pragma once

#include "PPhHelpers.h"

#pragma pack(push, 1)


namespace PPh
{
namespace CommonParams // Server - client common params
{
	constexpr int32_t PROTOCOL_VERSION = 1;
	constexpr int32_t DEFAULT_BUFLEN = 512;
	constexpr uint16_t CLIENT_UDP_PORT_START = 50000;
	constexpr uint16_t MAX_CLIENTS = 10;
	constexpr int32_t OBSERVER_EYE_SIZE = 16; // pixels
}
namespace MsgType
{
	enum MsgType
	{
		// client to server
		CheckVersion = 0,
		GetStatistics,
		GetState,
		GetStateExt,
		RotateLeft,
		RotateRight,
		RotateUp,
		RotateDown,
		MoveForward,
		MoveBackward,
		ClientToServerEnd, // !!!Always last
		// server to client
		CheckVersionResponse,
		SocketBusyByAnotherObserver,
		GetStatisticsResponse,
		GetStateResponse,
		GetStateExtResponse,
		SendPhoton
	};
}

class MsgBase
{
public:
	explicit MsgBase(uint8_t type) : m_type(type) {}
	const char* GetBuffer() const { return (const char*)this;	}
	uint8_t m_type;
};
//**************************************************************************************
//************************************** Client ****************************************
//**************************************************************************************
class MsgCheckVersion : public MsgBase
{
public:
	MsgCheckVersion() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::CheckVersion; }
	uint32_t m_clientVersion;
	uint64_t m_observerId;
};

class MsgGetStatistics: public MsgBase
{
public:
	MsgGetStatistics() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::GetStatistics; }
};

class MsgGetState : public MsgBase
{
public:
	MsgGetState() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::GetState; }
};

class MsgGetStateExt : public MsgBase
{
public:
	MsgGetStateExt() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::GetStateExt; }
};

class MsgRotateLeft : public MsgBase
{
public:
	MsgRotateLeft() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::RotateLeft; }

	uint8_t m_value;
};

class MsgRotateRight : public MsgBase
{
public:
	MsgRotateRight() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::RotateRight; }

	uint8_t m_value;
};

class MsgRotateUp : public MsgBase
{
public:
	MsgRotateUp() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::RotateUp; }

	uint8_t m_value;
};

class MsgRotateDown : public MsgBase
{
public:
	MsgRotateDown() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::RotateDown; }

	uint8_t m_value;
};

class MsgMoveForward : public MsgBase
{
public:
	MsgMoveForward() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::MoveForward; }

	uint8_t m_value;
};

class MsgMoveBackward : public MsgBase
{
public:
	MsgMoveBackward() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::MoveBackward; }

	uint8_t m_value;
};
//**************************************************************************************
//************************************** Server ****************************************
//**************************************************************************************
class MsgCheckVersionResponse : public MsgBase
{
public:
	MsgCheckVersionResponse() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::CheckVersionResponse; }
	uint32_t m_serverVersion;
	uint64_t m_observerId;
};

class MsgSocketBusyByAnotherObserver : public MsgBase
{
public:
	MsgSocketBusyByAnotherObserver() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::SocketBusyByAnotherObserver; }
	uint32_t m_serverVersion;
};

class MsgGetStatisticsResponse : public MsgBase
{
public:
	MsgGetStatisticsResponse() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::GetStatisticsResponse; }
	uint16_t m_universeThreadsCount;
	uint32_t m_fps; // quantum of time per second
	uint32_t m_observerThreadTickTime; // in microseconds
	uint32_t m_universeThreadMaxTickTime; // in microseconds
	uint32_t m_universeThreadMinTickTime; // in microseconds
	uint64_t m_clientServerPerformanceRatio; // in milli how much client ticks more often than server ticks
	uint64_t m_serverClientPerformanceRatio; // in milli how much server ticks more often than client ticks
};

class MsgGetStateResponse : public MsgBase
{
public:
	MsgGetStateResponse() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::GetStateResponse; }
	uint64_t m_time;
};

class MsgGetStateExtResponse : public MsgBase
{
public:
	MsgGetStateExtResponse() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::GetStateExtResponse; }
	VectorInt32Math m_pos;
	uint16_t m_movingProgress;
	int16_t m_latitude;
	int16_t m_longitude;
	uint32_t m_eatenCrumbNum;
	VectorInt32Math m_eatenCrumbPos;
};

class MsgSendPhoton : public MsgBase
{
public:
	MsgSendPhoton() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::SendPhoton; }
	EtherColor m_color;
	uint8_t m_posX;
	uint8_t m_posY;
};

template<class T>
const T* QueryMessage(const char *buf)
{
	if (buf[0] == T::GetType())
	{
		return (const T*)buf;
	}
	return nullptr;
}

}

#pragma pack(pop)
