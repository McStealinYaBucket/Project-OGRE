/* Copyright 2003-2006 ROBLOX Corporation, All Rights Reserved */
#include "stdafx.h"

#include "Util/NormalId.h"
#include "Util/Axes.h"
#include "Util/Faces.h"
#include "util/utilities.h"
#include "rbx/Debug.h"


namespace RBX {
	NormalIdMask normalIdToMask(NormalId normal)	// 0x232C3C (table dword_C37A90)
	{
		switch(normal){
		case NORM_X:	 return NORM_X_MASK;
		case NORM_Y:	 return NORM_Y_MASK;
		case NORM_Z:	 return NORM_Z_MASK;
		case NORM_X_NEG: return NORM_X_NEG_MASK;
		case NORM_Y_NEG: return NORM_Y_NEG_MASK;
		case NORM_Z_NEG: return NORM_Z_NEG_MASK;
		default:		 break;
		}
		return NORM_NONE_MASK;
	}

	bool validNormalId(NormalId normalId)	// 0x232C5A
	{
		return ((normalId >= 0) && (normalId < 6));
	}

	NormalId intToNormalId(int i)
	{
		return static_cast<NormalId>(i);
	}

	NormalId normalIdOpposite(NormalId normalId)	// 0x232C70
	{
		return (NormalId)((normalId + 3) % 6);
	}

	// STUB: PLAYER 0x008c6cd0
	// STUB: STUDIO 0x009c9ab0
	NormalId normalIdToU(NormalId normalId)	// 0x232C92 (assert at NormalId.cpp line 55)
	{
		switch ( normalId )
		{
			case NORM_X:	 return NORM_Z;
			case NORM_Y:	 return NORM_X;
			case NORM_Z:	 return NORM_Y;
			case NORM_X_NEG: return NORM_Z_NEG;
			case NORM_Y_NEG: return NORM_X_NEG;
			case NORM_Z_NEG: return NORM_Y_NEG;
			default:
				RBXASSERT(0);
				return NORM_Y;
		}
	}

	NormalId normalIdToV(NormalId normalId)
	{
		switch ( normalId )
		{
			case NORM_X:	 return NORM_Y;
			case NORM_Y:	 return NORM_Z;
			case NORM_Z:	 return NORM_X;
			case NORM_X_NEG: return NORM_Y_NEG;
			case NORM_Y_NEG: return NORM_Z_NEG;
			case NORM_Z_NEG: return NORM_X_NEG;
			default:
				RBXASSERT(0);
				return NORM_Y;
		}
	}

	////////////////////////////////////////////////////
	//
	// UVW to Object coords

	template<>
	Vector3 uvwToObject<NORM_X>(const Vector3& v)
	{
		return Vector3(v.z, v.y, -v.x);
	}

	template<>
	Vector3 uvwToObject<NORM_Y>(const Vector3& v)
	{
		return Vector3(-v.x, v.z, v.y);
	}

	template<>
	Vector3 uvwToObject<NORM_Z>(const Vector3& v)
	{
		return Vector3(v.x, v.y, v.z);
	}

	template<>
	Vector3 uvwToObject<NORM_X_NEG>(const Vector3& v)
	{
		return Vector3(-v.z, v.y, v.x);
	}

	template<>
	Vector3 uvwToObject<NORM_Y_NEG>(const Vector3& v)
	{
		return Vector3(v.x, -v.z, v.y);
	}

	template<>
	Vector3 uvwToObject<NORM_Z_NEG>(const Vector3& v)
	{
		return Vector3(-v.x, v.y, -v.z);
	}

	////////////////////////////////////////////////////
	//
	// Object to UVW coords

	template<>
	Vector3 objectToUvw<NORM_X>(const Vector3& v)
	{
		return Vector3(-v.z, v.y, v.x);
	}

	template<>
	Vector3 objectToUvw<NORM_Y>(const Vector3& v)
	{
		return Vector3(-v.x, v.z, v.y);
	}

	template<>
	Vector3 objectToUvw<NORM_Z>(const Vector3& v)
	{
		return Vector3(v.x, v.y, v.z);
	}

	template<>
	Vector3 objectToUvw<NORM_X_NEG>(const Vector3& v)
	{
		return Vector3(v.z, v.y, -v.x);
	}

	template<>
	Vector3 objectToUvw<NORM_Y_NEG>(const Vector3& v)
	{
		return Vector3(v.x, v.z, -v.y);
	}

	template<>
	Vector3 objectToUvw<NORM_Z_NEG>(const Vector3& v)
	{
		return Vector3(-v.x, v.y, -v.z);
	}

	// Given a point in object coords and a normal Id, returns the Vector3 where:
	// x:  U
	// y:  V
	// z:  W (out // away from the face)
	// in the coordinate system of the Face given by the NormalId
	// Right hand rule applies
	//
	// For +/- X and Z faces, (front, back, left, right) - V always points up
	// For +/- Y faces (top, bottom) - V always points to the back (+z)

	Vector3 uvwToObject(const Vector3& uvwPt, NormalId faceId)
	{
		switch (faceId)
		{
		case NORM_X:	 return uvwToObject<NORM_X>(uvwPt);
		case NORM_Y:	 return uvwToObject<NORM_Y>(uvwPt);
		case NORM_Z:	 return uvwToObject<NORM_Z>(uvwPt);
		case NORM_X_NEG: return uvwToObject<NORM_X_NEG>(uvwPt);
		case NORM_Y_NEG: return uvwToObject<NORM_Y_NEG>(uvwPt);
		case NORM_Z_NEG: return uvwToObject<NORM_Z_NEG>(uvwPt);
		default:
			RBXASSERT(0);
			return Vector3::unitX();
		}
	}

	Vector3 objectToUvw(const Vector3& objectPt, NormalId faceId)
	{
		switch (faceId)
		{
		case NORM_X:	 return objectToUvw<NORM_X>(objectPt);
		case NORM_Y:	 return objectToUvw<NORM_Y>(objectPt);
		case NORM_Z:	 return objectToUvw<NORM_Z>(objectPt);
		case NORM_X_NEG: return objectToUvw<NORM_X_NEG>(objectPt);
		case NORM_Y_NEG: return objectToUvw<NORM_Y_NEG>(objectPt);
		case NORM_Z_NEG: return objectToUvw<NORM_Z_NEG>(objectPt);
		default:
			RBXASSERT(0);
			return Vector3::unitX();
		}
	}

	//////////////////////////////////////////////////
	//////////////////////////////////////////////////

	template<NormalId normalId>
	Vector3 normalIdToVector3Internal()
	{
		RBXASSERT(validNormalId(normalId));
		Vector3 answer;
		int xyz = normalId % 3;
		answer[xyz] = (normalId < NORM_X_NEG) ? 1.0f : -1.0f;
		return answer;
	}

	// STUB: PLAYER 0x008c7760
	// STUB: STUDIO 0x009ca540
	const Vector3& normalIdToVector3(NormalId normalId)	// 0x233054 (assert at NormalId.cpp line 268)
	{
		switch (normalId)
		{
		case NORM_X:
			{
				static Vector3 v(normalIdToVector3Internal<NORM_X>());
				return v;
			}
		case NORM_Y:
			{
				static Vector3 v(normalIdToVector3Internal<NORM_Y>());
				return v;
			}
		case NORM_Z:
			{
				static Vector3 v(normalIdToVector3Internal<NORM_Z>());
				return v;
			}
		case NORM_X_NEG:
			{
				static Vector3 v(normalIdToVector3Internal<NORM_X_NEG>());
				return v;
			}
		case NORM_Y_NEG:
			{
				static Vector3 v(normalIdToVector3Internal<NORM_Y_NEG>());
				return v;
			}
		case NORM_Z_NEG:
			{
				static Vector3 v(normalIdToVector3Internal<NORM_Z_NEG>());
				return v;
			}
		default:
			{
				RBXASSERT(0);
				return Vector3::zero();
			}
		}
	}

	// STUB: PLAYER 0x008c7020
	// STUB: STUDIO 0x009c9e00
	Matrix3 normalIdToMatrix3Internal(NormalId normalId)	// 0x2332B4 (asserts at NormalId.cpp lines 280-282)
	{
		Vector3 uInObject = uvwToObject(Vector3::unitX(), normalId);
		Vector3 vInObject = uvwToObject(Vector3::unitY(), normalId);
		Vector3 wInObject = uvwToObject(Vector3::unitZ(), normalId);

		RBXASSERT(uInObject == uvwToObject(objectToUvw(uInObject, normalId), normalId));
		RBXASSERT(vInObject == uvwToObject(objectToUvw(vInObject, normalId), normalId));
		RBXASSERT(wInObject == uvwToObject(objectToUvw(wInObject, normalId), normalId));

		return Matrix3(	uInObject.x,	vInObject.x,	wInObject.x,
						uInObject.y,	vInObject.y,	wInObject.y,
						uInObject.z,	vInObject.z,	wInObject.z);
	}

	// STUB: PLAYER 0x008c7290
	// STUB: STUDIO 0x009ca070
	const Matrix3& normalIdToMatrix3(NormalId normalId)	// 0x2335CD (assert at NormalId.cpp line 319)
	{
		switch (normalId)
		{
		case NORM_X:
			{
				static Matrix3 x = normalIdToMatrix3Internal(NORM_X);		return x;
			}
		case NORM_Y:
			{
				static Matrix3 y = normalIdToMatrix3Internal(NORM_Y);		return y;
			}
		case NORM_Z:
			{
				static Matrix3 z = normalIdToMatrix3Internal(NORM_Z);		return z;
			}
		case NORM_X_NEG:
			{
				static Matrix3 xn = normalIdToMatrix3Internal(NORM_X_NEG);	return xn;
			}
		case NORM_Y_NEG:
			{
				static Matrix3 yn = normalIdToMatrix3Internal(NORM_Y_NEG);	return yn;
			}
		case NORM_Z_NEG:
			{
				static Matrix3 zn = normalIdToMatrix3Internal(NORM_Z_NEG);	return zn;
			}
		default:
			{
				RBXASSERT(0);
				return Matrix3::identity();
			}
		}
	}

	NormalId Vector3ToNormalId(const Vector3& v)
	{
		RBXASSERT(		(v == Vector3::unitX())
				||	(v == Vector3::unitY())
				||	(v == Vector3::unitZ())
				||	(v == -Vector3::unitX())
				||	(v == -Vector3::unitY())
				||	(v == -Vector3::unitZ())	);

		if (v.x == 1.0f) {
			return NORM_X;
		}
		else if (v.y == 1.0f) {
			return NORM_Y;
		}
		else if (v.z == 1.0f) {
			return NORM_Z;
		}
		else if (v.x == -1.0f) {
			return NORM_X_NEG;
		}
		else if (v.y == -1.0f) {
			return NORM_Y_NEG;
		}
		else if (v.z == -1.0f) {
			return NORM_Z_NEG;
		}
		else {
			RBXASSERT(0);		// This function assumes a unit, orthogally aligned Vector3
			return NORM_UNDEFINED;
		}
	}

	// just use the Z axis of the matrix as the normal vector
	NormalId Matrix3ToNormalId(const Matrix3& m)
	{
		return Vector3ToNormalId( m.column(2) );
	}

	// LEGACY - Deprecated
	// old stuff - need to inspect and see if it is really objectToUvw or uvwToObject

	// Note - this is actually doing uvwToObject
	Vector3 mapToUvw_Legacy(const Vector3& ptInObject, NormalId faceId)
	{
		return uvwToObject(ptInObject, faceId);
	}

} // namespace

//////////////////////////////////////////////////////////////////////////////
// Axes / Faces (binary links these alongside NormalId; 2016 splits them into
// Axes.cpp / Faces.cpp. Included here so this unit is self-contained.)
//////////////////////////////////////////////////////////////////////////////

namespace RBX {

	// mac: 0x1E7F2E
	Axes::Axes(int axisMask)
		: axisMask(axisMask)
	{}

	// mac: 0x1E7F3C (table dword_C366A0)
	Vector3::Axis Axes::normalIdToAxis(NormalId normalId)
	{
		switch(normalId){
			case NORM_X:
			case NORM_X_NEG:
				return Vector3::X_AXIS;
			case NORM_Y:
			case NORM_Y_NEG:
				return Vector3::Y_AXIS;
			case NORM_Z:
			case NORM_Z_NEG:
				return Vector3::Z_AXIS;
			default:
				return Vector3::DETECT_AXIS;
		}
	}

	NormalId Axes::axisToNormalId(Vector3::Axis axis)
	{
		switch(axis){
			case Vector3::X_AXIS:
				return NORM_X;
			case Vector3::Y_AXIS:
				return NORM_Y;
			case Vector3::Z_AXIS:
				return NORM_Z;
			default:
				return NORM_UNDEFINED;
		}
	}

	int Axes::axisToMask(Vector3::Axis axis)
	{
		switch(axis)
		{
		case Vector3::X_AXIS:
		case Vector3::Y_AXIS:
		case Vector3::Z_AXIS:
			return (1<< (int)axis);
		default:
			return 0;
		}
	}

	void Axes::setAxisByNormalId(NormalId normalId, bool value)
	{
		if(value){
			axisMask |= axisToMask(normalIdToAxis(normalId));
		}
		else{
			axisMask &= ~axisToMask(normalIdToAxis(normalId));
		}
	}

	// mac: 0x1E7F8A
	bool Axes::getAxisByNormalId(NormalId normalId) const
	{
		return (axisMask & axisToMask(normalIdToAxis(normalId))) != 0;
	}

	void Axes::setAxis(Vector3::Axis axis, bool value)
	{
		if(value){
			axisMask |= axisToMask(axis);
		}
		else{
			axisMask &= ~axisToMask(axis);
		}
	}

	bool Axes::getAxis(Vector3::Axis axis) const
	{
		return (axisMask & axisToMask(axis)) != 0;
	}

	// mac: 0x1E7FEA
	// FUNCTION: PLAYER 0x0081caa0
	// FUNCTION: STUDIO 0x008ed9b0
	template<>
	std::string StringConverter<Axes>::convertToString(const Axes& value)
	{
		std::string seperator = "";
		std::string result = "";
		for(int axis = 0; axis < 3; axis++){
			if(value.getAxis((RBX::Vector3::Axis)axis)){
				result += seperator;
				switch((RBX::Vector3::Axis)axis){
					case RBX::Vector3::X_AXIS:		result += "X";	break;
					case RBX::Vector3::Y_AXIS:		result += "Y";	break;
					case RBX::Vector3::Z_AXIS:		result += "Z";	break;
					default: break;
				}
				seperator = ", ";
			}
		}
		return result;
	}

	// mac: 0x1E855E
	template<>
	bool StringConverter<Axes>::convertToValue(const std::string& text, Axes& value)
	{
		value.clear();

		size_t oldpos = 0;
		size_t pos = 0;
		while((pos = text.find(",", oldpos)) != std::string::npos){
			RBX::Vector3::Axis axis;
			if(!StringConverter<G3D::Vector3::Axis>::convertToValue(text.substr(oldpos, (pos - oldpos)), axis))
				return false;
			value.setAxis(axis, true);
			oldpos = pos + 1;
		}

		//Handle the last one
		RBX::Vector3::Axis axis;
		if(!StringConverter<G3D::Vector3::Axis>::convertToValue(text.substr(oldpos, (text.size() - oldpos)), axis))
			return false;
		value.setAxis(axis, true);

		return true;
	}

	// mac: 0x201920
	// FUNCTION: PLAYER 0x0081c670
	// FUNCTION: STUDIO 0x008ed840
	Faces::Faces(int normalIdMask)
		: normalIdMask(normalIdMask)
	{}

	// mac: 0x201930
	void Faces::setNormalId(NormalId normalId, bool value)
	{
		if(value){
			normalIdMask |= normalIdToMask(normalId);
		}
		else{
			normalIdMask &= ~normalIdToMask(normalId);
		}
	}

	// mac: 0x201958
	bool Faces::getNormalId(NormalId normalId) const
	{
		return (normalIdMask & (normalIdToMask(normalId))) != 0;
	}

	// mac: 0x20197C
	// STUB: PLAYER 0x0081c6e0
	// STUB: STUDIO 0x0087a180
	template<>
	std::string StringConverter<Faces>::convertToString(const Faces& value)
	{
		std::string seperator = "";
		std::string result = "";
		for(NormalId normal = NORM_X; normal != NORM_UNDEFINED; normal = (NormalId)(normal+1)){
			if(value.getNormalId(normal)){
				result += seperator;
				switch(normal){
					case NORM_Y:		result += "Top";	break;
					case NORM_Y_NEG:	result += "Bottom";	break;
					case NORM_Z:		result += "Back";	break;
					case NORM_Z_NEG:	result += "Front";	break;
					case NORM_X:		result += "Right";	break;
					case NORM_X_NEG:	result += "Left";	break;
					default: break;
				}
				seperator = ", ";
			}
		}
		return result;
	}

	// mac: 0x201B9C
	template<>
	bool StringConverter<Faces>::convertToValue(const std::string& text, Faces& value)
	{
		value.clear();

		size_t oldpos = 0;
		size_t pos = 0;
		while((pos = text.find(",", oldpos)) != std::string::npos){
			NormalId normal;
			if(!StringConverter<NormalId>::convertToValue(text.substr(oldpos, (pos - oldpos)), normal))
				return false;
			value.setNormalId(normal, true);
			oldpos = pos + 1;
		}

		//Handle the last one
		NormalId normal;
		if(!StringConverter<NormalId>::convertToValue(text.substr(oldpos, (text.size() - oldpos)), normal))
			return false;
		value.setNormalId(normal, true);

		return true;
	}

} // namespace

//////////////////////////////////////////////////////////////////////////////
// TODO - functions in Util2_NormalId.cpp.txt belonging to OTHER modules:
//   - RBX::Lua::Bridge<RBX::Axes>/<RBX::Faces> getValue/pushNewObject/on_index/
//     on_newindex/on_tostring/registerClass/on_gc/on_eq (Lua bridge
//     instantiations, ~0x1E7xxx-0x1E8xxx) -> Lua bridge / Reflection module
//   - rbx::implementation::typed_holder<RBX::Axes>/<RBX::Faces> singleton/
//     construct_func/destruct_func -> Reflection TypedDescriptor module
//   - RBX::Extents::faceCenter(NormalId) 0x2007D8 -> Extents module
//   - RBX::Face::fromExtentsSide(Extents, NormalId) 0x201476 -> Face module
//     (uses Extents::getFaceCorners)
//   - RBX::ArcHandles::setAxes 0x27074C -> ArcHandles module
//   - Reflection::PropDescriptor<ArcHandles, Axes> ctor/dtor -> Reflection module
//   - StringConverter<G3D::Vector3::Axis> and StringConverter<NormalId>
//     convertToValue specializations (Axes.cpp / EnumProperty.cpp in 2016)
//////////////////////////////////////////////////////////////////////////////
