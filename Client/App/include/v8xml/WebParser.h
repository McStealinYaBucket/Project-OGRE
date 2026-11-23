/* v8xml/WebParser.h -- October 2013 reconstruction.
   Parses web API responses (XML value trees and JSON) into Reflection values.

   iOS 10-29 (locals included) lists for WebParser: parseWebGenericResponse
   (istream and XmlElement overloads), loadTable, loadList, loadEntry,
   loadValue, parseWebJSONResponse and populateValueTableFromPtree.
   Dropped as 2016-only (rapidjson era, no trace in PLAYER): parseJSONTable,
   parseJSONArray, parseJSONObject, writeJSON and its NonJSONBehavior enum;
   WebParser.cpp keeps their bodies under #if 0.
   Kept although the 2013 symtab lacks them, because WebParser.cpp defines
   them: parseWebListResponse, and the JSON reader split into
   legacyParseWebJSONResponse / ptreeParseWebJSONResponse (2013 has one
   parseWebJSONResponse) with the JSONmutex the ptree path locks. */
#pragma once

#include "Reflection/Type.h"

class XmlElement;

namespace RBX {

class WebParser
{
	static boost::mutex JSONmutex;

public:
	static bool parseWebGenericResponse(std::istream& in, Reflection::Variant& out);
	static bool parseWebGenericResponse(const XmlElement* root, Reflection::Variant& out);
	static bool parseWebListResponse(std::istream& in, Reflection::ValueArray& out);

	static bool legacyParseWebJSONResponse(std::stringstream& json, shared_ptr<const Reflection::ValueTable>& out);
	static bool ptreeParseWebJSONResponse(std::stringstream& json, shared_ptr<const Reflection::ValueTable>& out);

protected:
	static bool loadTable(const XmlElement* e, Reflection::ValueMap& out);
	static bool loadList(const XmlElement* e, Reflection::ValueArray& out);
	static bool loadEntry(const XmlElement* e, std::string& name, Reflection::Variant& out);
	static bool loadValue(const XmlElement* e, Reflection::Variant& out);
};

} // namespace RBX
