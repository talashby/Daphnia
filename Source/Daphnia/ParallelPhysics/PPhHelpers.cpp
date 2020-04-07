
#include "PPhHelpers.h"
#include "vector"
#include "chrono"
#include "algorithm"
#include "random"
#include <chrono>
#include "atomic"

namespace PPh
{

std::random_device rd;
std::mt19937 e1(rd());
int32_t Rand32(int32_t iRandMax) // from [0; iRandMax-1]
{
	std::uniform_int_distribution<int32_t> dist(0, iRandMax - 1);
	return dist(e1);
}

// ---------------------------------------------------------------------------------
// ------------------------------ VectorInt8Math -----------------------------------
const VectorInt8Math VectorInt8Math::ZeroVector(0, 0, 0);

VectorInt8Math::VectorInt8Math(int8_t posX, int8_t posY, int8_t posZ) : VectorMath(posX, posY, posZ)
{}

std::vector<int8_t> s_randomUniverseNumbersInt8;
std::atomic<uint8_t> s_randomIndexInt8 = 0;

void VectorInt8Math::InitRandom()
{
	uint32_t seed = (uint32_t)std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	if (s_randomUniverseNumbersInt8.empty())
	{
		s_randomUniverseNumbersInt8.resize((PPH_INT_MAX + 1) * 2);
		for (int ii = 0; ii < PPH_INT_MAX + 1; ++ii)
		{
			s_randomUniverseNumbersInt8[ii] = ii;
		}
	}

	for (int8_t ii = PPH_INT_MAX; ii > 0; --ii)
	{
		// Pick a random index from 0 to i  
		int8_t jj = Rand32(ii + 1);
		std::swap(s_randomUniverseNumbersInt8[ii], s_randomUniverseNumbersInt8[jj]);
	}

	std::copy(s_randomUniverseNumbersInt8.begin(), s_randomUniverseNumbersInt8.begin() + PPH_INT_MAX + 1,
		s_randomUniverseNumbersInt8.begin() + PPH_INT_MAX + 1);
}

int8_t VectorInt8Math::GetRandomNumber()
{
	return Rand32(PPH_INT_MAX + 1);
	int8_t number = s_randomUniverseNumbersInt8[s_randomIndexInt8++];
	return number;
}
// ------------------------------ VectorInt8Math -----------------------------------
// ---------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------
// ------------------------------ VectorInt16Math -----------------------------------
const VectorInt16Math VectorInt16Math::ZeroVector(0, 0, 0);

VectorInt16Math::VectorInt16Math(int16_t posX, int16_t posY, int16_t posZ) : VectorMath(posX, posY, posZ)
{}

std::vector<int16_t> s_randomUniverseNumbersInt16;
std::atomic <uint16_t> s_randomIndexInt16 = 0;

void VectorInt16Math::InitRandom()
{
	uint32_t seed = (uint32_t)std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	if (s_randomUniverseNumbersInt16.empty())
	{
		s_randomUniverseNumbersInt16.resize((PPH_INT_MAX + 1) * 2);
		for (int ii = 0; ii < PPH_INT_MAX + 1; ++ii)
		{
			s_randomUniverseNumbersInt16[ii] = ii;
		}
	}

	for (int16_t ii = PPH_INT_MAX; ii > 0; --ii)
	{
		// Pick a random index from 0 to i  
		int16_t jj = Rand32(ii + 1);
		std::swap(s_randomUniverseNumbersInt16[ii], s_randomUniverseNumbersInt16[jj]);
	}

	std::copy(s_randomUniverseNumbersInt16.begin(), s_randomUniverseNumbersInt16.begin() + PPH_INT_MAX + 1,
		s_randomUniverseNumbersInt16.begin() + PPH_INT_MAX + 1);
}

int16_t VectorInt16Math::GetRandomNumber()
{
	int16_t number = s_randomUniverseNumbersInt16[s_randomIndexInt16++];
	return number;
}
// ------------------------------ VectorInt8Math -----------------------------------
// ---------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------
// ------------------------------ VectorInt32Math ----------------------------------
const VectorInt32Math VectorInt32Math::ZeroVector(0, 0, 0);
const VectorInt32Math VectorInt32Math::OneVector(1, 1, 1);

VectorInt32Math::VectorInt32Math(int32_t posX, int32_t posY, int32_t posZ) : VectorMath(posX, posY, posZ)
{}

std::vector<int> s_randomUniverseNumbersInt32;
std::atomic<int32_t> s_randomIndexInt32 = VectorInt32Math::PPH_INT_MAX;
bool s_isRandomGeneratedInt32 = false;
void VectorInt32Math::InitRandom()
{
	uint32_t seed = (uint32_t)std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);
	if (s_randomUniverseNumbersInt32.empty())
	{
		s_randomUniverseNumbersInt32.resize(PPH_INT_MAX);
		for (int ii = 0; ii < PPH_INT_MAX; ++ii)
		{
			s_randomUniverseNumbersInt32[ii] = ii;
		}
	}
}

void GenerateNumbersStepByStep()
{
	static int32_t ii = VectorInt32Math::PPH_INT_MAX - 1;
	// Pick a random index from 0 to i  
	int32_t jj = Rand32(ii + 1);
	std::swap(s_randomUniverseNumbersInt32[ii], s_randomUniverseNumbersInt32[jj]);
	--ii;
	if (0 >= ii)
	{
		s_isRandomGeneratedInt32 = true;
	}
}

int32_t VectorInt32Math::GetRandomNumber()
{
	int32_t number = s_randomUniverseNumbersInt32[s_randomIndexInt32++]; // WARN! Multithreading problems
	if (s_randomIndexInt32 > PPH_INT_MAX)
	{
		if (s_isRandomGeneratedInt32)
		{
			s_randomIndexInt32 = 0;
		}
		else
		{
			s_randomIndexInt32 = PPH_INT_MAX;
			return Rand32(PPH_INT_MAX);
		}
	}
	return number;
}
// ------------------------------ VectorInt32Math ----------------------------------
// ---------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------
// ------------------------------ VectorFloatMath ----------------------------------

VectorFloatMath::VectorFloatMath(float posX, float posY, float posZ) : VectorMath(posX, posY, posZ)
{}

// ------------------------------ VectorFloatMath ----------------------------------
// ---------------------------------------------------------------------------------

BoxIntMath::BoxIntMath(const VectorInt32Math &minVector, const VectorInt32Math &maxVector) : m_minVector(minVector), m_maxVector(maxVector)
{}

EtherColor::EtherColor(uint8_t colorR, uint8_t colorG, uint8_t colorB) : m_colorB(colorB), m_colorG(colorG), m_colorR(colorR), m_colorA(0)
{}

const PPh::EtherColor EtherColor::ZeroColor(0, 0, 0);

int64_t GetTimeMs()
{
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	return ms.count();
}

}