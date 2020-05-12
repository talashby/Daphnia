#pragma once

#include "ServerProtocol.h"

#pragma pack(push, 1)

#define ADMIN_TCP_PORT 27015
#define ADMIN_TCP_PORT_STR "27015"

namespace PPh
{
namespace MsgTypeAdmin
{
	enum MsgTypeAdmin
	{
		// client to server
		RegisterObserver = 0,
		GetNextCrumb,
		RegisterAdminObserver,
		// server to client
		GetNextCrumbResponse
	};
}


//**************************************************************************************
//************************************** Client ****************************************
//**************************************************************************************
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
	uint64_t m_observerId;
};
//**************************************************************************************
//************************************** Server ****************************************
//**************************************************************************************
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
