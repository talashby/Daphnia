#pragma once

#include <stdint.h>

namespace PPh
{
	constexpr int8_t PPH_INT_MAX = 127;
	constexpr int8_t PPH_INT_MIN = -127;

	class VectorIntMath
	{
	public:
		static const VectorIntMath ZeroVector;
		static const VectorIntMath OneVector;

		VectorIntMath() = default;
		VectorIntMath(int8_t posX, int8_t posY, int8_t posZ);

		__forceinline VectorIntMath operator+(const VectorIntMath& V) const
		{
			return VectorIntMath(m_posX + V.m_posX, m_posY + V.m_posY, m_posZ + V.m_posZ);
		}

		__forceinline VectorIntMath operator*=(const int8_t& scale)
		{
			m_posX *= scale; m_posY *= scale; m_posZ *= scale;
			return *this;
		}

		__forceinline bool operator!=(const VectorIntMath& V) const
		{
			return m_posX != V.m_posX || m_posY != V.m_posY || m_posZ != V.m_posZ;
		}

		union
		{
			struct { int8_t m_posX, m_posY, m_posZ, m_tmp; };
			uint32_t AlignmentDummy;
		};
	};

	class VectorInt32Math
	{
	public:
		static const VectorInt32Math ZeroVector;

		VectorInt32Math() = default;
		VectorInt32Math(int32_t posX, int32_t posY, int32_t posZ);

		__forceinline VectorInt32Math operator+(const VectorInt32Math& V) const
		{
			return VectorInt32Math(m_posX + V.m_posX, m_posY + V.m_posY, m_posZ + V.m_posZ);
		}

		__forceinline VectorInt32Math operator*=(const int32_t& scale)
		{
			m_posX *= scale; m_posY *= scale; m_posZ *= scale;
			return *this;
		}

		__forceinline bool operator!=(const VectorInt32Math& V) const
		{
			return m_posX != V.m_posX || m_posY != V.m_posY || m_posZ != V.m_posZ;
		}

		int32_t m_posX;
		int32_t m_posY;
		int32_t m_posZ;
	};

	class BoxIntMath
	{
	public:
		BoxIntMath() = default;
		BoxIntMath(const VectorInt32Math &minVector, const VectorInt32Math &maxVector);

		VectorInt32Math m_minVector = VectorInt32Math::ZeroVector;
		VectorInt32Math m_maxVector = VectorInt32Math::ZeroVector;
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

	namespace RandomUniverse
	{
		void Init();
		int8_t GetRandomNumber(); // from 0 to PPH_INT_MAX
	};

	__forceinline int sign(int8_t x)
	{
		return (x > 0) - (x < 0);
	}

	int64_t GetTimeMs();
}
