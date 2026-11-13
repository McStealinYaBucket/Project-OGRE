#include "stdafx.h"

#include "Util/Region3.h"
#include "Util/Extents.h"
#include "Util/Utilities.h"
#include "Reflection/Type.h"

namespace RBX {

// mac: 0x62DF10 - guard byte_FB97E8 / static TType<Region3>("Region3") at dword_FB97C8
namespace Reflection
{
	// FUNCTION: PLAYER 0x007fa820
	// FUNCTION: STUDIO 0x008ee370
	template<>
	const Type& Type::getSingleton<Region3>()
	{
		static TType<Region3> type("Region3");
		return type;
	}
}

// mac: 0x62DF88 - default-constructs cframe, zeroes size
Region3::Region3()
{}

// mac: 0x62E014 - cframe rotation = Matrix3::identity(), translation = extents center, size = hi-lo
// FUNCTION: PLAYER 0x007fa5e0
// FUNCTION: STUDIO 0x008ee130
void Region3::init(const Extents &extents)
{
	cframe = CoordinateFrame(extents.center());
	size = extents.size();
}

// mac: 0x62DFB4 - builds Extents(min, max) on the stack then forwards to init
// FUNCTION: PLAYER 0x007fa780
// FUNCTION: STUDIO 0x008ee2d0
Region3::Region3(const Vector3& min, const Vector3& max)
{
	init(Extents(min, max));
}

// mac: 0x62E124
// FUNCTION: PLAYER 0x007fa7f0
// FUNCTION: STUDIO 0x008ee340
Region3::Region3(const Extents &extents)
{
	init(extents);
}

// mac: 0x62E154 - translation - size*0.5
Vector3 Region3::minPos() const
{
	return cframe.translation - (size * 0.5f);
}

// mac: 0x62E1B0 - translation + size*0.5
Vector3 Region3::maxPos() const
{
	return cframe.translation + (size * 0.5f);
}

// StringConverter<Region3> (section 232): 2016 v8tree/Property.cpp text
// adapted to G3D types (no 2013 EA banked for the text itself, loud
// DELTA); the composition shape (CF + "; " + Size) matches binary
// mac: 0x6F05F8-era patterns. convertToValue returns false like 2016.
template<>
std::string StringConverter<Region3>::convertToString(const Region3& value)
{
	std::string result = StringConverter<G3D::CoordinateFrame>::convertToString(value.getCFrame());
	result += "; ";
	result += StringConverter<G3D::Vector3>::convertToString(value.getSize());
	return result;
}

template<>
bool StringConverter<Region3>::convertToValue(const std::string& text, Region3& value)
{
	return false;
}

}