#pragma once

#include "PPhHelpers.h"

namespace PPh
{

struct EtherType
{
	enum EEtherType
	{
		Space = 0,
		Crumb,
		Block
	};
};

class ParallelPhysics
{
public:

	static bool Init(const VectorIntMath &universeSize); // returns true if success
	static ParallelPhysics* GetInstance();

	bool InitEtherCell(const VectorIntMath &pos, EtherType::EEtherType type, const EtherColor &color); // returns true if success

	const VectorIntMath & GetUniverseSize() const;

	void StartSimulation();
	void StopSimulation();

private:
	ParallelPhysics();

	VectorIntMath m_universeSize = VectorIntMath::ZeroVector;
};



	struct Observer
	{
		void Init(const VectorIntMath &position, const VectorIntMath &direction);

		Observer* GetInstance();
	};
}
