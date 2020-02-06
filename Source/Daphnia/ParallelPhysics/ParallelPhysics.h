#pragma once

namespace ParallelPhysics
{
	bool Init(int32_t dimensionX, int32_t dimensionY, int32_t dimensionZ); // returns true if success

	int32_t GetDimensionX();
	int32_t GetDimensionY();
	int32_t GetDimensionZ();

	struct EtherType
	{
		enum EEtherType
		{
			Space = 0,
			Crumb,
			Block
		};
	};
	bool InitEtherCell(int32_t xPos, int32_t yPos, int32_t zPos, EtherType::EEtherType type, int32_t colorR, int32_t colorG, int32_t colorB); // returns true if success
}
