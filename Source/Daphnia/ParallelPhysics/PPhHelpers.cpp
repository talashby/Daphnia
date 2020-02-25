
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

VectorIntMath::VectorIntMath(int8_t posX, int8_t posY, int8_t posZ) : m_posX(posX), m_posY(posY), m_posZ(posZ)
{}

const VectorInt32Math VectorInt32Math::ZeroVector(0, 0, 0);

VectorInt32Math::VectorInt32Math(int32_t posX, int32_t posY, int32_t posZ) : m_posX(posX), m_posY(posY), m_posZ(posZ)
{}

BoxIntMath::BoxIntMath(const VectorInt32Math &minVector, const VectorInt32Math &maxVector) : m_minVector(minVector), m_maxVector(maxVector)
{}

EtherColor::EtherColor() : m_colorB(0), m_colorG(0), m_colorR(0), m_colorA(0)
{}


EtherColor::EtherColor(int8_t colorR, int8_t colorG, int8_t colorB) : m_colorB(colorR), m_colorG(colorG), m_colorR(colorB), m_colorA(0)
{}


std::vector<int8_t> s_randomUniverseNumbers;
int8_t s_randomIndex = 0;

std::random_device rd;
std::mt19937 e1(rd());
int32_t Rand32(int32_t iRandMax) // from [0; iRandMax-1]
{
	std::uniform_int_distribution<int32_t> dist(0, iRandMax - 1);
	return dist(e1);
}

void RandomUniverse::Init()
{
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	if (s_randomUniverseNumbers.empty())
	{
		s_randomUniverseNumbers.resize((PPH_INT_MAX+1)*2);
		for (int ii = 0; ii < PPH_INT_MAX+1; ++ii)
		{
			s_randomUniverseNumbers[ii] = ii;
		}
	}

	for (int8_t ii = PPH_INT_MAX; ii > 0; --ii)
	{
		// Pick a random index from 0 to i  
		int8_t jj = Rand32(ii + 1);
		std::swap(s_randomUniverseNumbers[ii], s_randomUniverseNumbers[jj]);
	}
	
	std::copy(s_randomUniverseNumbers.begin(), s_randomUniverseNumbers.begin() + PPH_INT_MAX + 1, s_randomUniverseNumbers.begin() + PPH_INT_MAX + 1);
}

int8_t RandomUniverse::GetRandomNumber()
{
	int8_t number = s_randomUniverseNumbers[s_randomIndex];
	++s_randomIndex;
	return number;
}

int64_t GetTimeMs()
{
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	return ms.count();
}

}