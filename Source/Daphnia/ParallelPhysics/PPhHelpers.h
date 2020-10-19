#pragma once

#include <stdint.h>

namespace PPh
{
	typedef class VectorInt8Math OrientationVectorMath;

	template<class T, class D>
	class VectorMath
	{
	public:
		static const D ZeroVector;

		VectorMath() = default;
		VectorMath(T posX, T posY, T posZ) : m_posX(posX), m_posY(posY), m_posZ(posZ)
		{}

		__forceinline D operator+(const VectorMath& V) const
		{
			return D(m_posX + V.m_posX, m_posY + V.m_posY, m_posZ + V.m_posZ);
		}

		__forceinline D operator-(const VectorMath& V) const
		{
			return D(m_posX - V.m_posX, m_posY - V.m_posY, m_posZ - V.m_posZ);
		}

		__forceinline VectorMath operator*=(const T& scale)
		{
			m_posX *= scale; m_posY *= scale; m_posZ *= scale;
			return *this;
		}

		__forceinline bool operator!=(const VectorMath& V) const
		{
			return m_posX != V.m_posX || m_posY != V.m_posY || m_posZ != V.m_posZ;
		}

		union
		{
			struct { T m_posX, m_posY, m_posZ; };
			T m_posArray[3];
			uint32_t AlignmentDummy;
		};
	};

	class VectorInt8Math : public VectorMath<int8_t, VectorInt8Math>
	{
	public:
		static const int8_t PPH_INT_MAX = 127;
		static const int8_t PPH_INT_MIN = -PPH_INT_MAX;

		VectorInt8Math() = default;
		VectorInt8Math(int8_t posX, int8_t posY, int8_t posZ);

		static void InitRandom();
		static int8_t GetRandomNumber(); // from 0 to PPH_INT_MAX
	};

	class VectorInt16Math : public VectorMath<int16_t, VectorInt16Math>
	{
	public:
		static const int16_t PPH_INT_MAX = 32767;
		static const int16_t PPH_INT_MIN = -PPH_INT_MAX;

		VectorInt16Math() = default;
		VectorInt16Math(int16_t posX, int16_t posY, int16_t posZ);

		static void InitRandom();
		static int16_t GetRandomNumber(); // from 0 to PPH_INT_MAX
	};

	class VectorInt32Math : public VectorMath<int32_t, VectorInt32Math>
	{
	public:
		static const int32_t PPH_INT_MAX = 1073741824;
		static const int32_t PPH_INT_MIN = -PPH_INT_MAX;
		static const VectorInt32Math OneVector;

		VectorInt32Math() = default;
		VectorInt32Math(int32_t posX, int32_t posY, int32_t posZ);

		static void InitRandom();
		static int32_t GetRandomNumber(); // from 0 to PPH_INT_MAX
	};

	class VectorFloatMath : public VectorMath<float, VectorFloatMath>
	{
	public:
		VectorFloatMath() = default;
		VectorFloatMath(float posX, float posY, float posZ);
	};

	class BoxIntMath
	{
	public:
		BoxIntMath() = default;
		BoxIntMath(const VectorInt32Math &minVector, const VectorInt32Math &maxVector);

		VectorInt32Math m_minVector = VectorInt32Math::ZeroVector;
		VectorInt32Math m_maxVector = VectorInt32Math::ZeroVector;
	};

	struct EtherType
	{
		enum EEtherType
		{
			Space = 0,
			Crumb,
			Block,
			Observer
		};
	};

	class EtherColor
	{
	public:
		EtherColor() = default;
		EtherColor(uint8_t colorR, uint8_t colorG, uint8_t colorB);
		EtherColor(uint8_t colorR, uint8_t colorG, uint8_t colorB, uint8_t colorA);

		static const EtherColor ZeroColor;

		union
		{
			struct { uint8_t m_colorB, m_colorG, m_colorR, m_colorA; };
			uint32_t AlignmentDummy;
		};

		__forceinline bool operator==(const EtherColor& V) const
		{
			return m_colorB == V.m_colorB && m_colorG == V.m_colorG && m_colorR == V.m_colorR && m_colorA == V.m_colorA;
		}
	};

	typedef int8_t PhotonParam; // warning! OBSERVER_EYE_SIZE should be max 16
	typedef int8_t DaphniaIdType; // max 256 daphnias on server
	struct Photon
	{
		Photon() = default;
		explicit Photon(const OrientationVectorMath &orientation) : m_orientation(orientation)
		{}
		EtherColor m_color;
		OrientationVectorMath m_orientation;
		PhotonParam m_param; // used to store coordinates of neuron which sent this photon
		DaphniaIdType m_param2; // used to store daphnia id who sent this photon
	};

	int64_t GetTimeMs();

	int32_t Rand32(int32_t iRandMax);

	template<class T>
	__forceinline int Sign(T x)
	{
		return (x > 0) - (x < 0);
	}

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
}
