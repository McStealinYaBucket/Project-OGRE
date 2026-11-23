/* v8xml/WebSerializer.h -- October 2013 reconstruction.
   Turns a Reflection value tree into the XML element tree the web API reads
   (the inverse of WebParser's XML path). iOS 10-29 lists all four static
   members with these parameter lists; PLAYER's writeValue is 0x0081f410. */
#pragma once

#include "Reflection/Type.h"

class XmlElement;

namespace RBX {

class WebSerializer
{
public:
	// Each returns a new element the caller owns.
	static XmlElement* writeValue(const Reflection::Variant& v);                          // scalar, table or list
	static XmlElement* writeTable(const Reflection::ValueMap& table);                     // name -> value
	static XmlElement* writeList(const Reflection::ValueArray& list);                     // ordered values
	static XmlElement* writeEntry(const std::string& name, const Reflection::Variant& v); // one table row
};

} // namespace RBX
