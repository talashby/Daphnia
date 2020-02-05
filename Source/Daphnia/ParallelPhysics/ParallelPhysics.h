#pragma once

namespace ParallelPhysics
{
	bool Init(int32_t xDimension, int32_t yDimension, int32_t zDimension); // returns true if success

	int32_t GetXDimension();
	int32_t GetYDimension();
	int32_t GetZDimension();

	bool InitEtherCell(int32_t xPos, int32_t yPos, int32_t zPos, int32_t colorR, int32_t colorG, int32_t colorB); // returns true if success
}
