/* Util/CellID.cpp -- October 2013 (first import, section 227).
   Evidence: class layout pinned by the bridge disasm (see CellID.h); the
     ctor/dtor bodies below are March 2016 Util/CellID.cpp carries (no 2013
     ctor/dtor EAs banked -- loud DELTA). The StringConverter pair adapts
     the 2016 v8tree/Property.cpp shapes to G3D::Vector3 (location converter
     already ported in v8tree/EnumProperty.cpp); convertToValue returns
     false like 2016. convertToString has no banked 2013 EA (carried text).
   Original file: Client/App/util/CellID.cpp */

#include "stdafx.h"

#include "Util/CellID.h"
#include "Util/Utilities.h"

namespace RBX
{
	CellID::CellID(bool newIsNil, float newLocation[3], shared_ptr<Instance> newTerrainPart)
	{
		isNil = newIsNil;
		location = G3D::Vector3(newLocation[0], newLocation[1], newLocation[2]);
		terrainPart = newTerrainPart;
	}

	CellID::CellID()
	{
		isNil = true;
		location = G3D::Vector3(0.0, 0.0, 0.0);
	}

	CellID::~CellID()
	{
	}

	template<>
	std::string StringConverter<CellID>::convertToString(const CellID& value)
	{
		std::string result = "{" + StringConverter<bool>::convertToString(value.getIsNil()) + "}";
		result += ", ";
		result += "{" + StringConverter<G3D::Vector3>::convertToString(value.getLocation()) + "}";
		return result;
	}

	template<>
	bool StringConverter<CellID>::convertToValue(const std::string& text, CellID& value)
	{
		return false;
	}
} // namespace RBX
