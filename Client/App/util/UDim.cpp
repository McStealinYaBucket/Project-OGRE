#include "stdafx.h"

#include "Util/UDim.h"
#include "util/Utilities.h"

using namespace RBX;

namespace RBX
{
	// mac: 0x2580CA
	template<>
	std::string StringConverter<UDim>::convertToString(const UDim& value)
	{
		std::string result = StringConverter<float>::convertToString(value.scale);
		result += ", ";
		result += StringConverter<int>::convertToString(value.offset);
		return result;
	}


	// mac: 0x2581B0 - parses "scale,offset" (separator may be ',' or ';')
	template<>
	bool StringConverter<UDim>::convertToValue(const std::string& text, UDim& value)
	{
		int pos = text.find_first_of(",;", 0);
		if (pos<0)
			return false;
		value.scale = (float)atof(text.substr(0, pos).c_str());

		int last = pos+1;
		pos = text.length();
		if (pos<last)
			return false;
		value.offset = (int)atoi(text.substr(last, pos-last).c_str());

		return true;
	}


	// mac: 0x2582C4 - "{s,o}, {s,o}"
	// FUNCTION: PLAYER 0x008d3710
	// FUNCTION: STUDIO 0x0084a8b0
	template<>
	std::string StringConverter<UDim2>::convertToString(const UDim2& value)
	{
		std::string result = "{" + StringConverter<UDim>::convertToString(value.x) + "}";
		result += ", ";
		result += "{" + StringConverter<UDim>::convertToString(value.y) + "}";
		return result;
	}


	// mac: 0x258618 - parses "{s,o}, {s,o}"
	template<>
	bool StringConverter<UDim2>::convertToValue(const std::string& text, UDim2& value)
	{
		int openBracket = text.find_first_of("{", 0);
		if (openBracket < 0)
			return false;
		int closeBracket = text.find_first_of("}", openBracket);
		if(closeBracket < 0)
			return false;
		if(!StringConverter<UDim>::convertToValue(text.substr(openBracket+1, closeBracket-openBracket-1), value[0]))
			return false;

		int last = closeBracket+1;
		openBracket = text.find_first_of("{", last);
		if (openBracket < 0)
			return false;
		closeBracket = text.find_first_of("}", openBracket);
		if(closeBracket < 0)
			return false;
		if(!StringConverter<UDim>::convertToValue(text.substr(openBracket+1, closeBracket-openBracket-1), value[1]))
			return false;

		return true;
	}
}//namespace RBX

// transform(float) + operator*(float): declared in Util/UDim.h and called
// by UDim2::operator*(Vector2)/operator*(float) above, but never defined --
// the exe link surfaced them (LNK2001) once the Lua bridge pulled UDim.obj.
// No 2013 EAs banked for these two leaves (the banked EA series covers
// +,-,unm and the UDim2 block); 2016 util/UDim.cpp shapes carried:
// transform = offset + scale*value (offset int16 promotes to float).
float UDim::transform(const float value) const
{
	return this->scale*value + this->offset;
}

UDim UDim::operator*(float rhs) const
{
	return UDim(this->scale*rhs, this->offset*rhs);
}

// mac: 0x258818
UDim UDim::operator+ (const UDim& v) const
{
	return UDim(scale+v.scale, offset+v.offset);
}
// mac: 0x25883A
UDim UDim::operator- (const UDim& v) const
{
	return UDim(scale-v.scale, offset-v.offset);
}
// mac: 0x25885C
UDim UDim::operator- () const
{
	return UDim(-scale, -offset);
}


// mac: 0x258874 - result = Vector2(x.transform(rhs.x), y.transform(rhs.y)),
// i.e. offset + scale*component (see on_index/decomp: int16 offset added to float product)
G3D::Vector2 UDim2::operator*(const G3D::Vector2 rhs) const
{
	return G3D::Vector2(this->x.transform(rhs.x), this->y.transform(rhs.y));
}
// mac: 0x2588BA
UDim2 UDim2::operator* (float v) const
{
	return UDim2(x*v, y*v);
}


// mac: 0x258918
UDim2 UDim2::operator+ (const UDim2& v) const
{
	return UDim2(x+v.x, y+v.y);
}
// mac: 0x258962
UDim2 UDim2::operator- (const UDim2& v) const
{
	return UDim2(x-v.x, y-v.y);
}
// mac: 0x2589AC
UDim2 UDim2::operator- () const
{
	return UDim2(-x, -y);
}

// TODO (not part of Util/UDim module; belong to Lua bridge / v8datamodel headers, captured in
// exports/decomp/Util_UDim.cpp.txt because UDim/UDim2 are their template args):
//  - RBX::Lua::Bridge<UDim/UDim2,true>::getValue/pushNewObject/on_index/on_newindex/on_gc/on_eq/
//    registerClass (0x1633FC..0x198A48) -> Lua bridge template instantiation (Lua/Bridge.h)
//    Note: 2013 on_index exposes "Scale"/"Offset" (UDim) and "X"/"Y"/"Width"/"Height" (UDim2).
//  - rbx::implementation::typed_holder<UDim2>::construct_func (0x1654C0)
//  - RBX::GuiObject tween family + TweenInterpolate (0x3A7B6C..0x3AB4B0) -> v8datamodel/GuiObject
//    (asserts reference /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/GuiObject.cpp
//    lines 329, 357, 556, 605)
//  - RBX::BillboardGui::setSize (0x2837DA) -> v8datamodel/BillboardGui
//  - Reflection::PropDescriptor/TypedPropertyDescriptor/BoundFuncDesc/RemoteEventDesc and
//    EventReplicatorBase<GuiObject, void(UDim2)> instantiations -> reflection headers
//  - boost::function2<void, GuiObject*, UDim2>::operator() (0x3AFF70) -> boost header
