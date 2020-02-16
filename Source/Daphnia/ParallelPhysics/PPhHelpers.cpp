
#include "PPhHelpers.h"
#include "vector"
#include "chrono"
#include "algorithm"
#include "random"

namespace PPh
{
const VectorIntMath VectorIntMath::ZeroVector(0, 0, 0);

VectorIntMath::VectorIntMath(int32_t posX, int32_t posY, int32_t posZ) : m_posX(posX), m_posY(posY), m_posZ(posZ)
{}

EtherColor::EtherColor() : m_colorB(0), m_colorG(0), m_colorR(0), m_colorA(0)
{}


EtherColor::EtherColor(int8_t colorR, int8_t colorG, int8_t colorB) : m_colorB(colorR), m_colorG(colorG), m_colorR(colorB), m_colorA(0)
{}


std::vector<int> s_randomUniverseNumbers;
int32_t s_randomIndex = 0;
void RandomUniverse::Init()
{
	if (s_randomUniverseNumbers.empty())
	{
		s_randomUniverseNumbers.resize(PPH_INT_MAX + 1);
		for (int ii = 0; ii < PPH_INT_MAX+1; ++ii)       // add 0-99 to the vector
		{
			s_randomUniverseNumbers[ii] = ii;
		}
		for (int ii = PPH_INT_MAX; ii > 0; --ii)
		{
			// Pick a random index from 0 to i  
			int jj = rand() % (ii + 1);

			std::swap(s_randomUniverseNumbers[ii], s_randomUniverseNumbers[jj]);
		}
		//unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		//std::shuffle(s_randomUniverseNumbers.begin(), s_randomUniverseNumbers.end(), std::default_random_engine(seed));
	}
}

int32_t RandomUniverse::GetRandomNumber()
{
	int32_t number = s_randomUniverseNumbers[s_randomIndex];
	++s_randomIndex;
	if (s_randomIndex > PPH_INT_MAX)
	{
		s_randomIndex = 0;
	}
	return number;
}

}