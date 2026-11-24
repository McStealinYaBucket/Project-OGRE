#include "stdafx.h"

#include "RbxAssert.h"
#include "V8Xml/XmlElement.h"
#include "Reflection/Object.h"
#include "V8Xml/XmlSerializer.h"
#include "Util/Guid.h"
#include "Util/Utilities.h"
#include "rbx/Debug.h"
#include <stdlib.h>
#include <stdio.h>


using std::string;
using std::vector;
using namespace G3D;

using namespace RBX;

// The tag definitions (and their GLOBAL markers) are at the end of the file.










































































































///////////////////////////////////////////////////////////////////////////

// FUNCTION: PLAYER 0x00613c70
// FUNCTION: STUDIO 0x006da780
bool XmlNameValuePair::isValueEqual(const RBX::Name* value) const {
	switch (valueType) {
	case STRING:
		return *value==*stringValue;
	case NAME:
		return *value==*nameValue;
	default:
		return false;
	}
}

// FUNCTION: PLAYER 0x00613cb0
// FUNCTION: STUDIO 0x006da7c0
template<>
bool XmlNameValuePair::isValueType<ContentId>() const
{
	return valueType==CONTENTID;
}

// FUNCTION: PLAYER 0x00613cc0
// FUNCTION: STUDIO 0x006da7d0
template<>
bool XmlNameValuePair::isValueType<std::string>() const
{
	return valueType==STRING;
}

// FUNCTION: PLAYER 0x00613cd0
// FUNCTION: STUDIO 0x006da7e0
bool XmlNameValuePair::getValue(std::string& value) const
{
	if (valueType==STRING) {
		value = *stringValue;
		return true;
	}
	if (valueType==NAME) {
		value = nameValue ? nameValue->c_str() : "";
		return true;
	}
	return false;
}

// FUNCTION: PLAYER 0x00613d40
// FUNCTION: STUDIO 0x006da850
const XmlElement* XmlElement::findFirstChildByTag(const XmlTag& _tag) const {
	for (const XmlElement* child = firstChild(); child!=NULL; child = child->nextSibling())
		if (child->getTag()==_tag)
			return child;
	return NULL;
}

// FUNCTION: PLAYER 0x00613d60
// FUNCTION: STUDIO 0x006da870
const XmlElement* XmlElement::findNextChildWithSameTag(const XmlElement* node) const {
	for (const XmlElement* child = node->nextSibling(); child!=NULL; child = child->nextSibling())
		if (child->getTag()==node->getTag())
			return child;
	return NULL;
}

// FUNCTION: PLAYER 0x00613dc0
// FUNCTION: STUDIO 0x006da890
const XmlAttribute* XmlElement::findAttribute(const XmlTag& _tag) const {
	for (const XmlAttribute* attribute = getFirstAttribute(); attribute!=NULL; attribute = getNextAttribute(attribute))
		if (attribute->getTag()==_tag)
			return attribute;
	return NULL;
}

// FUNCTION: PLAYER 0x00613de0
// FUNCTION: STUDIO 0x006da8f0
void XmlNameValuePair::clearValue() const {
	switch (valueType) {
	case STRING:
		delete stringValue;
		break;
	case CONTENTID:
		delete contentIdValue;
		break;
	case HANDLE:
		delete handleValue;
		break;
    default:
        break;
	}
	valueType = NONE;
}

// FUNCTION: PLAYER 0x00613e70
// FUNCTION: STUDIO 0x006da980
bool XmlNameValuePair::getValue(const RBX::Name*& value) const
{
	if (valueType==NAME) {
		value = nameValue;
		return true;
	}
	if (valueType==STRING) {
		value = RBX::Name::declare(stringValue->c_str());
		clearValue();
		nameValue = value;
		valueType = NAME;
		return true;
	}
	return false;
}

// FUNCTION: PLAYER 0x00613ee0
// FUNCTION: STUDIO 0x006da9f0
bool XmlNameValuePair::getValue(RBX::ContentId& value) const
{
	if (valueType==CONTENTID) {
		value = *contentIdValue;
		return true;
	}
	if (valueType==STRING) {
		value = ContentId(*stringValue);
		clearValue();
		contentIdValue = new ContentId(value);
		valueType = CONTENTID;
		return true;
	}
	return false;
}

// FUNCTION: PLAYER 0x00614000
// FUNCTION: STUDIO 0x006dab10
bool XmlNameValuePair::getValue(int& value) const
{
	if (valueType==INT) {
		value = intValue;
		return true;
	}
	if (valueType==STRING) {
		if (StringConverter<int>::convertToValue(*stringValue, value)) {
			clearValue();
			intValue = value;
			valueType = INT;
			return true;
		}
	}
	return false;
}

// FUNCTION: PLAYER 0x00614060
// FUNCTION: STUDIO 0x006dab70
bool XmlNameValuePair::getValue(unsigned int& value) const
{
	if (valueType==UINT) {
		value = uintValue;
		return true;
	}
	if (valueType==STRING) {
		if (StringConverter<unsigned int>::convertToValue(*stringValue, value)) {
			clearValue();
			uintValue = value;
			valueType = UINT;
			return true;
		}
	}
	return false;
}

// FUNCTION: PLAYER 0x006140c0
// FUNCTION: STUDIO 0x006dabd0
bool XmlNameValuePair::getValue(bool& value) const
{
	if (valueType==BOOL) {
		value = boolValue;
		return true;
	}
	if (valueType==STRING) {
		if (StringConverter<bool>::convertToValue(*stringValue, value)) {
			clearValue();
			boolValue = value;
			valueType = BOOL;
			return true;
		}
	}
	return false;
}

// FUNCTION: PLAYER 0x00614120
// FUNCTION: STUDIO 0x006dac30
bool XmlNameValuePair::getValue(float& value) const
{
	if (valueType==FLOAT) {
		value = floatValue;
		return true;
	}
	if (valueType==STRING) {
		if (RBX::StringConverter<float>::convertToValue(*stringValue, value)) {
			clearValue();
			floatValue = value;
			valueType = FLOAT;
			return true;
		}
	}
	RBXASSERT(valueType!=DOUBLE);	// No provision (yet) for converting from double back to float

	return false;
}

// FUNCTION: PLAYER 0x006141c0
// FUNCTION: STUDIO 0x006dacd0
bool XmlNameValuePair::getValue(double& value) const
{
	if (valueType==DOUBLE) {
		value = doubleValue;
		return true;
	}

	if (valueType==FLOAT) {
		value = (double)floatValue;
		clearValue();
		doubleValue = value;
		valueType = DOUBLE;
		return true;
	}

	if (valueType==STRING) {
		if (RBX::StringConverter<double>::convertToValue(*stringValue, value)) {
			clearValue();
			doubleValue = value;
			valueType = DOUBLE;
			return true;
		}
	}

	return false;
}

// FUNCTION: PLAYER 0x00614230
// FUNCTION: STUDIO 0x006dad40
bool XmlElement::isXsiNil() const {
	const XmlAttribute* xnil = findAttribute(*name_xsinil);
	bool isNil;
	return xnil!=NULL && xnil->getValue(isNil) && isNil;
}

// STUB: PLAYER 0x00614270
// STUB: STUDIO 0x006dad80
bool XmlNameValuePair::getValue(RBX::InstanceHandle &value) const {
	
	if (valueType==NAME && *nameValue==*value_IDREF_null) {
		clearValue();
		handleValue = new RBX::InstanceHandle(NULL);
		valueType = HANDLE;
	} else if (valueType==STRING && *value_IDREF_null==*stringValue) {
		clearValue();
		handleValue = new RBX::InstanceHandle(NULL);
		valueType = HANDLE;
	} else if (valueType==STRING && *stringValue=="") {
		// legacy files didn't use the "null" keyword
		clearValue();
		handleValue = new RBX::InstanceHandle(NULL);
		valueType = HANDLE;
	}

	if (valueType==HANDLE) {
		value = *handleValue;
		return true;
	} 
	else
		return false;
}



// STUB: PLAYER 0x00614a30
// STUB: STUDIO 0x006db540
std::string XmlNameValuePair::toString(XmlWriter* writer) const
{
	switch (valueType)
	{
	case BOOL:
		return RBX::StringConverter<bool>::convertToString(boolValue);
	case INT:
		return RBX::StringConverter<int>::convertToString(intValue);
	case UINT:
		return RBX::StringConverter<unsigned int>::convertToString(uintValue);
	case FLOAT:
		return RBX::StringConverter<float>::convertToString(floatValue);
	case DOUBLE:
		return RBX::StringConverter<double>::convertToString(doubleValue);
	case NAME:
		return nameValue->str;

	case HANDLE:
		if (handleValue->getTarget()==NULL)
			return value_IDREF_null->str;
		else {
			// 2013 (iOS 10-29 XmlNameValuePair::toString): sprintf "RBX%d" of the
			// writer's handle index; there is no per-object xmlId in 2013.
			char buffer[32];
			sprintf(buffer, "RBX%d", writer->getHandleIndex(*handleValue));
			return buffer;
		}

	case STRING:
		return *stringValue;

	case CONTENTID:
		return contentIdValue->toString();
	case NONE:
		return "";
	default:
		RBXASSERT(false);
		return "";
	}
}

// Not emitted in the PLAYER XmlElement.obj range (unreferenced or folded):

template<>
bool XmlNameValuePair::isValueType<int>() const
{
	return valueType==INT;
}

template<>
bool XmlNameValuePair::isValueType<unsigned int>() const
{
	return valueType==UINT;
}

template<>
bool XmlNameValuePair::isValueType<bool>() const
{
	return valueType==BOOL;
}

template<>
bool XmlNameValuePair::isValueType<float>() const
{
	return valueType==FLOAT;
}

template<>
bool XmlNameValuePair::isValueType<double>() const
{
	return valueType==DOUBLE;
}

template<>
bool XmlNameValuePair::isValueType<const RBX::Name*>() const
{
	return valueType==NAME;
}

template<>
bool XmlNameValuePair::isValueType<RBX::InstanceHandle>() const
{
	return valueType==HANDLE;
}

XmlAttribute* XmlElement::findAttribute(const XmlTag& _tag) {
	for (XmlAttribute* attribute = getFirstAttribute(); attribute!=NULL; attribute = getNextAttribute(attribute))
		if (attribute->getTag()==_tag)
			return attribute;
	return NULL;
}


// Oct 2013 explicit instantiation (MergeBinder nil checks): compare against the
// pointed-to Name via the non-template overload.

template<>
bool XmlNameValuePair::isValueEqual<const RBX::Name* const*>(const RBX::Name* const* value) const {
	return isValueEqual(*value);
}

///////////////////////////////////////////////////////////////////////////
// Tag names, in their 2013 initialisation order.

// The 2016 CustomPhysics tags do not exist yet: XmlElement.obj's 56 initializers (PLAYER
// 0x00a63d30 on) end with xsi:noNamespaceSchemaLocation.
// GLOBAL: PLAYER 0x00de28a8
// GLOBAL: STUDIO 0x00f67560
const RBX::Name* const value_IDREF_null = Name::declare("null");
// GLOBAL: PLAYER 0x00de2960
// GLOBAL: STUDIO 0x00f67610
const RBX::Name* const value_IDREF_nil = Name::declare("nil");

// GLOBAL: PLAYER 0x00de2928
// GLOBAL: STUDIO 0x00f675d8
const XmlTag* const name_xsinil = Name::declare("xsi:nil");
// GLOBAL: PLAYER 0x00de28bc
// GLOBAL: STUDIO 0x00f67574
const XmlTag* const name_xsitype = Name::declare("xsi:type");
// GLOBAL: PLAYER 0x00de2878
// GLOBAL: STUDIO 0x00f67528
const XmlTag* const tag_xmlnsxsi = Name::declare("xmlns:xsi");

// TODO: Put these in a file that knows about the Roblox schema
// GLOBAL: PLAYER 0x00de287c
// GLOBAL: STUDIO 0x00f6752c
const XmlTag* const name_root = Name::declare("root");
// GLOBAL: PLAYER 0x00de2890
// GLOBAL: STUDIO 0x00f67544
const XmlTag* const name_referent = Name::declare("referent");
// GLOBAL: PLAYER 0x00de2938
// GLOBAL: STUDIO 0x00f675e8
const XmlTag* const tag_roblox = Name::declare("roblox");
// GLOBAL: PLAYER 0x00de2908
// GLOBAL: STUDIO 0x00f675c0
const XmlTag* const tag_version = Name::declare("version");
// GLOBAL: PLAYER 0x00de2910
// GLOBAL: STUDIO 0x00f675c8
const XmlTag* const tag_assettype = Name::declare("assettype");
// GLOBAL: PLAYER 0x00de2900
// GLOBAL: STUDIO 0x00f675b8
const XmlTag* const tag_External = Name::declare("External");
// GLOBAL: PLAYER 0x00de289c
// GLOBAL: STUDIO 0x00f67550
const XmlTag* const name_Ref = Name::declare("Ref");
// GLOBAL: PLAYER 0x00de28ec
// GLOBAL: STUDIO 0x00f675a4
const XmlTag* const name_token = Name::declare("token");
// GLOBAL: PLAYER 0x00de28cc
// GLOBAL: STUDIO 0x00f67584
const XmlTag* const name_name = Name::declare("name");
// GLOBAL: PLAYER 0x00de290c
// GLOBAL: STUDIO 0x00f675c4
const XmlTag* const tag_Refs = Name::declare("Refs");
// GLOBAL: PLAYER 0x00de28c4
// GLOBAL: STUDIO 0x00f6757c
const XmlTag* const tag_X = Name::declare("X");
// GLOBAL: PLAYER 0x00de2954
// GLOBAL: STUDIO 0x00f67604
const XmlTag* const tag_Y = Name::declare("Y");
// GLOBAL: PLAYER 0x00de28a0
// GLOBAL: STUDIO 0x00f67558
const XmlTag* const tag_Z = Name::declare("Z");

// GLOBAL: PLAYER 0x00de28f0
// GLOBAL: STUDIO 0x00f675a8
const XmlTag* const tag_R00 = Name::declare("R00");
// GLOBAL: PLAYER 0x00de291c
// GLOBAL: STUDIO 0x00f675cc
const XmlTag* const tag_R01 = Name::declare("R01");
// GLOBAL: PLAYER 0x00de2880
// GLOBAL: STUDIO 0x00f67530
const XmlTag* const tag_R02 = Name::declare("R02");
// GLOBAL: PLAYER 0x00de28b8
// GLOBAL: STUDIO 0x00f67570
const XmlTag* const tag_R10 = Name::declare("R10");
// GLOBAL: PLAYER 0x00de28f8
// GLOBAL: STUDIO 0x00f675b0
const XmlTag* const tag_R11 = Name::declare("R11");
// GLOBAL: PLAYER 0x00de28e8
// GLOBAL: STUDIO 0x00f675a0
const XmlTag* const tag_R12 = Name::declare("R12");
// GLOBAL: PLAYER 0x00de28b4
// GLOBAL: STUDIO 0x00f6756c
const XmlTag* const tag_R20 = Name::declare("R20");
// GLOBAL: PLAYER 0x00de28ac
// GLOBAL: STUDIO 0x00f67564
const XmlTag* const tag_R21 = Name::declare("R21");
// GLOBAL: PLAYER 0x00de2884
// GLOBAL: STUDIO 0x00f67534
const XmlTag* const tag_R22 = Name::declare("R22");

// GLOBAL: PLAYER 0x00de2924
// GLOBAL: STUDIO 0x00f675d4
const XmlTag* const tag_R = Name::declare("R");
// GLOBAL: PLAYER 0x00de28d0
// GLOBAL: STUDIO 0x00f67588
const XmlTag* const tag_G = Name::declare("G");
// GLOBAL: PLAYER 0x00de28d4
// GLOBAL: STUDIO 0x00f6758c
const XmlTag* const tag_B = Name::declare("B");
// GLOBAL: PLAYER 0x00de293c
// GLOBAL: STUDIO 0x00f675ec
const XmlTag* const tag_class = Name::declare("class");
// GLOBAL: PLAYER 0x00de2904
// GLOBAL: STUDIO 0x00f675bc
const XmlTag* const tag_Item = Name::declare("Item");
// GLOBAL: PLAYER 0x00de2888
// GLOBAL: STUDIO 0x00f67538
const XmlTag* const tag_Properties = Name::declare("Properties");
// GLOBAL: PLAYER 0x00de28e0
// GLOBAL: STUDIO 0x00f67598
const XmlTag* const tag_Feature = Name::declare("Feature");

// GLOBAL: PLAYER 0x00de2968
// GLOBAL: STUDIO 0x00f67618
const XmlTag* const tag_hash = Name::declare("hash");
// GLOBAL: PLAYER 0x00de2964
// GLOBAL: STUDIO 0x00f67614
const XmlTag* const tag_null = Name::lookup("null");		// already declared elsewhere
// GLOBAL: PLAYER 0x00de2944
// GLOBAL: STUDIO 0x00f675f4
const XmlTag* const tag_mimeType = Name::declare("mimeType");

// GLOBAL: PLAYER 0x00de2894
// GLOBAL: STUDIO 0x00f67548
const XmlTag* const tag_S = Name::declare("S");
// GLOBAL: PLAYER 0x00de28fc
// GLOBAL: STUDIO 0x00f675b4
const XmlTag* const tag_O = Name::declare("O");

// GLOBAL: PLAYER 0x00de28dc
// GLOBAL: STUDIO 0x00f67594
const XmlTag* const tag_XS = Name::declare("XS");
// GLOBAL: PLAYER 0x00de2958
// GLOBAL: STUDIO 0x00f67608
const XmlTag* const tag_XO = Name::declare("XO");
// GLOBAL: PLAYER 0x00de28c8
// GLOBAL: STUDIO 0x00f67580
const XmlTag* const tag_YS = Name::declare("YS");
// GLOBAL: PLAYER 0x00de2920
// GLOBAL: STUDIO 0x00f675d0
const XmlTag* const tag_YO = Name::declare("YO");

// GLOBAL: PLAYER 0x00de2950
// GLOBAL: STUDIO 0x00f67600
const XmlTag* const tag_faces = Name::declare("faces");
// GLOBAL: PLAYER 0x00de28e4
// GLOBAL: STUDIO 0x00f6759c
const XmlTag* const tag_axes = Name::declare("axes");

// GLOBAL: PLAYER 0x00de28b0
// GLOBAL: STUDIO 0x00f67568
const XmlTag* const tag_Origin = Name::declare("origin");
// GLOBAL: PLAYER 0x00de288c
// GLOBAL: STUDIO 0x00f6753c
const XmlTag* const tag_Direction = Name::declare("direction");

// GLOBAL: PLAYER 0x00de2930
// GLOBAL: STUDIO 0x00f675e0
const XmlTag* const tag_Min = Name::declare("min");
// GLOBAL: PLAYER 0x00de2940
// GLOBAL: STUDIO 0x00f675f0
const XmlTag* const tag_Max = Name::declare("max");

// GLOBAL: PLAYER 0x00de292c
// GLOBAL: STUDIO 0x00f675dc
const XmlTag* const tag_WebTable = Name::declare("Table");
// GLOBAL: PLAYER 0x00de2934
// GLOBAL: STUDIO 0x00f675e4
const XmlTag* const tag_WebList = Name::declare("List");
// GLOBAL: PLAYER 0x00de2898
// GLOBAL: STUDIO 0x00f6754c
const XmlTag* const tag_WebEntry = Name::declare("Entry");
// GLOBAL: PLAYER 0x00de2874
// GLOBAL: STUDIO 0x00f67524
const XmlTag* const tag_WebKey = Name::declare("Key");
// GLOBAL: PLAYER 0x00de294c
// GLOBAL: STUDIO 0x00f675fc
const XmlTag* const tag_WebValue = Name::declare("Value");
// GLOBAL: PLAYER 0x00de295c
// GLOBAL: STUDIO 0x00f6760c
const XmlTag* const tag_WebType = Name::declare("Type");


// GLOBAL: PLAYER 0x00de28c0
// GLOBAL: STUDIO 0x00f67578
const XmlTag* const tag_xsinoNamespaceSchemaLocation = Name::declare("xsi:noNamespaceSchemaLocation");

// Sizes proven by the 100% getValue family, findAttribute, isValueEqual and writeOpenTag.
#include "decomp.h"
DECOMP_SIZE_ASSERT(XmlNameValuePair, 0x10);
DECOMP_SIZE_ASSERT(XmlAttribute, 0x18);
DECOMP_SIZE_ASSERT(XmlElement, 0x28);
