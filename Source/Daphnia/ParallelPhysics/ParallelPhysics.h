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
		Block,
		Observer
	};
};

class ParallelPhysics
{
public:

	static bool Init(const VectorIntMath &universeSize); // returns true if success
	static ParallelPhysics* GetInstance();

	bool InitEtherCell(const VectorIntMath &pos, EtherType::EEtherType type, const EtherColor &color = EtherColor()); // returns true if success

	const VectorIntMath & GetUniverseSize() const;

	void StartSimulation();
	void StopSimulation();

private:
	ParallelPhysics();

	VectorIntMath m_universeSize = VectorIntMath::ZeroVector;
};



class Observer
{
public:
	void Init(const VectorIntMath &position, const VectorIntMath &orientation);

	static Observer* GetInstance();

private:
	VectorIntMath m_orientation = VectorIntMath::ZeroVector;
};
}
