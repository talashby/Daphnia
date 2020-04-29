#pragma once

#include "PPhHelpers.h"
#include <string>

class AActor;

namespace PPh
{

namespace AdminUniverse // used to load server universe data for client administrator
{
	bool Init(const VectorInt32Math &universeSize); // returns true if success. threadsCount 0 means simulate near observer
	bool SaveUniverse(const std::string &fileName);

	const VectorInt32Math &GetUniverseSize();

	bool InitEtherCell(const VectorInt32Math &pos, EtherType::EEtherType type, const EtherColor &color = EtherColor()); // returns true if success

	bool GetNextCrumb(VectorInt32Math &outCrumbPos, EtherColor &outCrumbColor);
	void EtherCellSetCrumbActor(const VectorInt32Math &pos, AActor *crumbActor);
	AActor* EtherCellGetCrumbActor(const VectorInt32Math &pos);
}

namespace AdminTcp
{
	bool Connect();
	void LoadCrumbs();
	void Disconnect();
} // namespace AdminTcp

} // namespace PPh
