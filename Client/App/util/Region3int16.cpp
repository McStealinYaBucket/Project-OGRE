/* Util/Region3int16.cpp -- October 2013 (new TU, section 232).
   The Region3int16 class itself is header-inline (Util/Region3int16.h);
   this TU homes its StringConverter pair. Text carried from March 2016
   v8tree/Property.cpp (no 2013 EA banked for the text, loud DELTA);
   convertToValue returns false like 2016.
   Original file: n/a (2013 companion; 2016 has App/util/Region3int16.cpp
   with different contents). */

#include "stdafx.h"

#include "Util/Region3int16.h"
#include "Util/UDim.h"
#include "Util/Utilities.h"

namespace RBX {

	// StringConverter<Vector3int16> (section 232): 2016 v8tree/Property.cpp
	// text (no 2013 EA banked, loud DELTA). Needed by the Region3int16
	// converter below.
	template<>
	std::string StringConverter<RBX::Vector3int16>::convertToString(const RBX::Vector3int16& value)
	{
		std::string result = StringConverter<int>::convertToString(value[0]);
		result += ", ";
		result += StringConverter<int>::convertToString(value[1]);
		result += ", ";
		result += StringConverter<int>::convertToString(value[2]);
		return result;
	}

	template<>
	bool StringConverter<RBX::Vector3int16>::convertToValue(const std::string& text, G3D::Vector3int16& value)
	{
		int pos = (int)text.find_first_of(",;", 0);
		if (pos < 0)
			return false;
		value[0] = (G3D::int16)atoi(text.substr(0, pos).c_str());
		int last = pos + 1;
		pos = (int)text.find_first_of(",;", last);
		if (pos < last)
			return false;
		value[1] = (G3D::int16)atoi(text.substr(last, pos - last).c_str());
		last = pos + 1;
		pos = (int)text.length();
		if (pos < last)
			return false;
		value[2] = (G3D::int16)atoi(text.substr(last, pos - last).c_str());
		return true;
	}

	template<>
	std::string StringConverter<RBX::Vector2int16>::convertToString(const RBX::Vector2int16& value)
	{
		std::string result = StringConverter<int>::convertToString(value[0]);
		result += ", ";
		result += StringConverter<int>::convertToString(value[1]);
		return result;
	}

	template<>
	bool StringConverter<RBX::Vector2int16>::convertToValue(const std::string& text, G3D::Vector2int16& value)
	{
		int pos = (int)text.find_first_of(",;", 0);
		if (pos < 0)
			return false;
		value[0] = (G3D::int16)atoi(text.substr(0, pos).c_str());
		int last = pos + 1;
		pos = (int)text.length();
		if (pos < last)
			return false;
		value[1] = (G3D::int16)atoi(text.substr(last, pos - last).c_str());
		return true;
	}


	template<>
	std::string StringConverter<Region3int16>::convertToString(const Region3int16& value)
	{
		std::string result = StringConverter<RBX::Vector3int16>::convertToString(value.getMinPos());
		result += "; ";
		result += StringConverter<RBX::Vector3int16>::convertToString(value.getMaxPos());
		return result;
	}

	template<>
	bool StringConverter<Region3int16>::convertToValue(const std::string& text, Region3int16& value)
	{
		return false;
	}

} // namespace RBX
