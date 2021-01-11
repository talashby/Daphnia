#pragma once

#include "ServerProtocol.h"

#pragma pack(push, 1)

#define ADMIN_TCP_PORT 27015
#define ADMIN_TCP_PORT_STR "27015"

namespace PPh
{
constexpr int32_t ADMIN_PROTOCOL_VERSION = 1;

namespace MsgTypeAdmin
{
	enum MsgTypeAdmin
	{
		// client to server
		CheckVersion = 0,
		GetNextCrumb,
		RegisterAdminObserver,
		// server to client
		CheckVersionResponse,
		GetNextCrumbResponse
	};
}


//**************************************************************************************
//************************************** Client ****************************************
//**************************************************************************************
class MsgAdminCheckVersion : public MsgBase
{
public:
	MsgAdminCheckVersion() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgTypeAdmin::CheckVersion; }
	uint32_t m_clientVersion;
};

class MsgAdminGetNextCrumb : public MsgBase
{
public:
	MsgAdminGetNextCrumb() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgTypeAdmin::GetNextCrumb; }
};

class MsgRegisterAdminObserver : public MsgBase
{
public:
	MsgRegisterAdminObserver() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgTypeAdmin::RegisterAdminObserver; }
	uint64_t m_adminObserverId; // used to send other daphnias position
};
//**************************************************************************************
//************************************** Server ****************************************
//**************************************************************************************
class MsgAdminCheckVersionResponse : public MsgBase
{
public:
	MsgAdminCheckVersionResponse() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgTypeAdmin::CheckVersionResponse; }
	uint32_t m_serverVersion;
	uint32_t m_universeScale;
};

class MsgAdminGetNextCrumbResponse : public MsgBase
{
public:
	MsgAdminGetNextCrumbResponse() : MsgBase(GetType()) {}
	static uint8_t GetType() { return MsgTypeAdmin::GetNextCrumbResponse; }
	EtherColor m_color;
	uint32_t m_posX;
	uint32_t m_posY;
	uint32_t m_posZ;
};

}

#pragma pack(pop)
