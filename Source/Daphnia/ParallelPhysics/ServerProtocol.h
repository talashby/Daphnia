#pragma once

#include "PPhHelpers.h"

#pragma pack(push, 1)

namespace PPh
{
	constexpr int32_t MAX_PROTOCOL_BUFFER_SIZE = 256;

namespace MsgType
{
	enum MsgType
	{
		// client to server
		GetVersion = 0,
		GetState,
		// server to client
		SendVersion,
		SendState,
		SendPhoton
	};
}

class MsgBase
{
public:
	explicit MsgBase(uint8_t type) : m_type(type) {}
	const char* GetBuffer() { return (const char*)this;	}
	uint8_t m_type;
};

class MsgGetState : public MsgBase
{
public:
	MsgGetState() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::GetState; }
};

class MsgSendState : public MsgBase
{
public:
	MsgSendState() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::SendState; }
	uint64_t m_time;
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
T* QueryMessage(char *buf)
{
	if (buf[0] == T::GetType())
	{
		return (T*)buf;
	}
	return nullptr;
}

}

#pragma pack(pop)
