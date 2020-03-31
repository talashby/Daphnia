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
		RotateLeft,
		RotateRight,
		RotateUp,
		RotateDown,
		MoveForward,
		MoveBackward,
		AdminGetNextCrumb,
		// server to client
		SendVersion,
		SendState,
		SendPhoton,
		AdminSendNextCrumb
	};
}

class MsgBase
{
public:
	explicit MsgBase(uint8_t type) : m_type(type) {}
	const char* GetBuffer() { return (const char*)this;	}
	uint8_t m_type;
};
//**************************************************************************************
//************************************** Client ****************************************
//**************************************************************************************
class MsgGetState : public MsgBase
{
public:
	MsgGetState() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::GetState; }
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

class MsgAdminGetNextCrumb : public MsgBase
{
public:
	MsgAdminGetNextCrumb() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::AdminGetNextCrumb; }
};
//**************************************************************************************
//************************************** Server ****************************************
//**************************************************************************************
class MsgSendState : public MsgBase
{
public:
	MsgSendState() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::SendState; }
	uint64_t m_time;
	int16_t m_latitude;
	int16_t m_longitude;
	bool m_isEatenCrumb;
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

class MsgAdminSendNextCrumb : public MsgBase
{
public:
	MsgAdminSendNextCrumb() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgType::AdminSendNextCrumb; }
	EtherColor m_color;
	uint32_t m_posX;
	uint32_t m_posY;
	uint32_t m_posZ;
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
