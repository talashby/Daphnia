#pragma once

#include <stdint.h>

namespace PPh
{
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
