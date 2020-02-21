
#include "PPhHelpers.h"
#include "vector"
#include "chrono"
#include "algorithm"
#include "random"
#include <chrono>

namespace PPh
{
const VectorIntMath VectorIntMath::ZeroVector(0, 0, 0);
const VectorIntMath VectorIntMath::OneVector(1, 1, 1);

VectorIntMath::VectorIntMath(int32_t posX, int32_t posY, int32_t posZ) : m_posX(posX), m_posY(posY), m_posZ(posZ)
{}

BoxIntMath::BoxIntMath(const VectorIntMath &minVector, const VectorIntMath &maxVector) : m_minVector(minVector), m_maxVector(maxVector)
{}

EtherColor::EtherColor() : m_colorB(0), m_colorG(0), m_colorR(0), m_colorA(0)
{}


EtherColor::EtherColor(int8_t colorR, int8_t colorG, int8_t colorB) : m_colorB(colorR), m_colorG(colorG), m_colorR(colorB), m_colorA(0)
{}


std::vector<int> s_randomUniverseNumbers;
int32_t s_randomIndex = PPH_INT_MAX;
bool s_isRandomGenerated = false;
void RandomUniverse::Init()
{
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	if (s_randomUniverseNumbers.empty())
	{
		s_randomUniverseNumbers.resize(PPH_INT_MAX);
		for (int ii = 0; ii < PPH_INT_MAX; ++ii)
		{
			s_randomUniverseNumbers[ii] = ii;
		}
	}
}

std::random_device rd;
std::mt19937 e1(rd());
int32 Rand32(int32 iRandMax) // from [0; iRandMax-1]
{
	check(iRandMax > 0);
	std::uniform_int_distribution<int32> dist(0, iRandMax - 1);
	return dist(e1);
}

void GenerateNumbersStepByStep()
{
	static int32_t ii = PPH_INT_MAX-1;
	// Pick a random index from 0 to i  
	int32_t jj = Rand32(ii + 1);
	std::swap(s_randomUniverseNumbers[ii], s_randomUniverseNumbers[jj]);
	--ii;
	if (0 >= ii)
	{
		s_isRandomGenerated = true;
	}
}

int32_t RandomUniverse::GetRandomNumber()
{
	int32_t number = s_randomUniverseNumbers[s_randomIndex];
	++s_randomIndex;
	if (s_randomIndex > PPH_INT_MAX)
	{
		if (s_isRandomGenerated)
		{
			s_randomIndex = 0;
		}
		else
		{
			s_randomIndex = PPH_INT_MAX;
			return Rand32(PPH_INT_MAX);
		}
	}
	return number;
}

int64_t GetTimeMs()
{

	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	return ms.count();
}

}