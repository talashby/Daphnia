#pragma once

#include <stdint.h>

namespace PPh
{
	constexpr int PPH_INT_MAX = 1073741824; // 2*MAX_INT-1 should be less or equal std::numeric_limits<int32_t>::max() because of arithmetics reasons
	constexpr int PPH_INT_MIN = -PPH_INT_MAX; // 2*MIN_INT should be less or equal std::numeric_limits<int32_t>::min() because of arithmetics reasons

	class VectorIntMath
	{
	public:
		static const VectorIntMath ZeroVector;

		VectorIntMath() = default;
		VectorIntMath(int32_t posX, int32_t posY, int32_t posZ);

		int32_t m_posX;
		int32_t m_posY;
		int32_t m_posZ;
	};

	class EtherColor
	{
	public:
		EtherColor();
		EtherColor(int8_t colorR, int8_t colorG, int8_t colorB);

		union
		{
			struct { uint8_t m_colorB, m_colorG, m_colorR, m_colorA; };
			uint32_t AlignmentDummy;
		};
	};

}
