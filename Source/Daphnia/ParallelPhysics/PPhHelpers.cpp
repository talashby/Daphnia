
#include "PPhHelpers.h"

namespace PPh
{
const VectorIntMath VectorIntMath::ZeroVector(0, 0, 0);

VectorIntMath::VectorIntMath(int32_t posX, int32_t posY, int32_t posZ) : m_posX(posX), m_posY(posY), m_posZ(posZ)
{}

EtherColor::EtherColor() : m_colorB(0), m_colorG(0), m_colorR(0), m_colorA(0)
{}


EtherColor::EtherColor(int8_t colorR, int8_t colorG, int8_t colorB) : m_colorB(colorR), m_colorG(colorG), m_colorR(colorB), m_colorA(0)
{}

}