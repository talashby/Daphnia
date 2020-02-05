
#include "ParallelPhysics.h"
#include "vector"

namespace ParallelPhysics
{
struct EtherColor
{
	EtherColor() : colorB(0), colorG(0), colorR(0), colorA(0)
	{}

	union { struct { uint8_t colorB, colorG, colorR, colorA; }; uint32_t AlignmentDummy; };
};

struct EtherCell
{
	bool m_isSolid = false;
	EtherColor m_color;
};

std::vector< std::vector< std::vector<EtherCell> > > universe;


bool Init(int32_t xDimension, int32_t yDimension, int32_t zDimension)
{
	if (0 < xDimension && 0 < yDimension && 0 < yDimension)
	{
		universe.resize(xDimension);
		for (auto &itY : universe)
		{
			itY.resize(yDimension);
			for (auto &itZ : itY)
			{
				itZ.resize(zDimension);
			}
		}
		return true;
	}
	return false;
}

int32_t GetXDimension()
{
	return universe.size();
}

int32_t GetYDimension()
{
	if (universe.size())
	{
		return universe[0].size();
	}
	return 0;
}

int32_t GetZDimension()
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

bool InitEtherCell(int32_t xPos, int32_t yPos, int32_t zPos, int32_t colorR, int32_t colorG, int32_t colorB)
{
	if (universe.size() > xPos)
	{
		if (universe[xPos].size() > yPos)
		{
			if (universe[xPos][yPos].size() > zPos)
			{
				EtherCell &cell = universe[xPos][yPos][zPos];
				cell.m_isSolid = true;
				cell.m_color.colorR = colorR;
				cell.m_color.colorG = colorG;
				cell.m_color.colorB = colorB;
				return true;
			}
		}
	}
	return false;
}

}