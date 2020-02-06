
#include "ParallelPhysics.h"
#include "vector"

namespace ParallelPhysics
{

struct EtherColor
{
	EtherColor() : m_colorB(0), m_colorG(0), m_colorR(0), m_colorA(0)
	{}

	union { struct { uint8_t m_colorB, m_colorG, m_colorR, m_colorA; }; uint32_t AlignmentDummy; };
};

struct EtherCell
{
	int32 m_type = EtherType::Space;
	EtherColor m_color;
};

std::vector< std::vector< std::vector<EtherCell> > > universe;


bool Init(int32_t dimensionX, int32_t dimensionY, int32_t dimensionZ)
{
	if (0 < dimensionX && 0 < dimensionY && 0 < dimensionZ)
	{
		universe.resize(dimensionX);
		for (auto &itY : universe)
		{
			itY.resize(dimensionY);
			for (auto &itZ : itY)
			{
				itZ.resize(dimensionZ);
			}
		}
		return true;
	}
	return false;
}

int32_t GetDimensionX()
{
	return universe.size();
}

int32_t GetDimensionY()
{
	if (universe.size())
	{
		return universe[0].size();
	}
	return 0;
}

int32_t GetDimensionZ()
{
	if (universe.size())
	{
		if (universe[0].size())
		{
			return universe[0][0].size();
		}
	}
	return 0;
}

bool InitEtherCell(int32_t xPos, int32_t yPos, int32_t zPos, EtherType::EEtherType type, int32_t colorR, int32_t colorG, int32_t colorB)
{
	if (universe.size() > xPos)
	{
		if (universe[xPos].size() > yPos)
		{
			if (universe[xPos][yPos].size() > zPos)
			{
				EtherCell &cell = universe[xPos][yPos][zPos];
				cell.m_type = type;
				cell.m_color.m_colorR = colorR;
				cell.m_color.m_colorG = colorG;
				cell.m_color.m_colorB = colorB;
				return true;
			}
		}
	}
	return false;
}

}