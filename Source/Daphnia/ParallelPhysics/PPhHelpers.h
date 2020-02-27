#pragma once

#include <stdint.h>

namespace PPh
{
	template<class T, class D>
	class VectorIntMath
	{
	public:
		static const D ZeroVector;

		VectorIntMath() = default;
		VectorIntMath(T posX, T posY, T posZ) : m_posX(posX), m_posY(posY), m_posZ(posZ)
		{}

		static __forceinline int Sign(T x)
		{
			return (x > 0) - (x < 0);
		}

		__forceinline D operator+(const VectorIntMath& V) const
		{
			return D(m_posX + V.m_posX, m_posY + V.m_posY, m_posZ + V.m_posZ);
		}

		__forceinline VectorIntMath operator*=(const T& scale)
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
			struct { T m_posX, m_posY, m_posZ, m_tmp; };
			uint32_t AlignmentDummy;
		};
	};

	class VectorInt8Math : public VectorIntMath<int8_t, VectorInt8Math>
	{
	public:
		static const int8_t PPH_INT_MAX = 127;
		static const int8_t PPH_INT_MIN = -PPH_INT_MAX;

		VectorInt8Math() = default;
		VectorInt8Math(int8_t posX, int8_t posY, int8_t posZ);

		static void InitRandom();
		static int8_t GetRandomNumber(); // from 0 to PPH_INT_MAX
	};

	class VectorInt16Math : public VectorIntMath<int16_t, VectorInt16Math>
	{
	public:
		static const int16_t PPH_INT_MAX = 32767;
		static const int16_t PPH_INT_MIN = -PPH_INT_MAX;

		VectorInt16Math() = default;
		VectorInt16Math(int16_t posX, int16_t posY, int16_t posZ);

		static void InitRandom();
		static int16_t GetRandomNumber(); // from 0 to PPH_INT_MAX
	};

	class VectorInt32Math : public VectorIntMath<int32_t, VectorInt32Math>
	{
	public:
		static const int32_t PPH_INT_MAX = 1073741824;
		static const int32_t PPH_INT_MIN = -PPH_INT_MAX;

		VectorInt32Math() = default;
		VectorInt32Math(int32_t posX, int32_t posY, int32_t posZ);

		static void InitRandom();
		static int32_t GetRandomNumber(); // from 0 to PPH_INT_MAX
	};

	typedef VectorInt32Math OrientationVectorMath;

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

	int64_t GetTimeMs();
}
