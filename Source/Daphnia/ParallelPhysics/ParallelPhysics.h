#pragma once

namespace ParallelPhysics
{
	bool Init(int32_t dimensionX, int32_t dimensionY, int32_t dimensionZ); // returns true if success

	int32_t GetDimensionX();
	int32_t GetDimensionY();
	int32_t GetDimensionZ();

	void StartSimulation();
	void StopSimulation();

	struct Observer
	{
		void Init(int32_t posX, int32_t posY, int32_t posZ, int32_t rotX, int32_t rotY, int32_t rotZ);

		Observer* GetInstance();
	};

	struct EtherType
	{
		enum EEtherType
		{
			Space = 0,
			Crumb,
			Block
		};
	};
	bool InitEtherCell(int32_t posX, int32_t posY, int32_t posZ, EtherType::EEtherType type, int32_t colorR, int32_t colorG, int32_t colorB); // returns true if success
}
