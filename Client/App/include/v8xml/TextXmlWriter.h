/* V8Xml/TextXmlWriter.h — October 2013 XML writer.
   Evidence: TextXmlWriter::encodedWrite 0x5BBB5C, writeOpenTag 0x5BBCC4,
     writeCloseTag 0x5BBF8A, serialize 0x5BD818/0x5BD83E, serializeNode 0x5BD8F6
     (F:\2013Decompile\exports\decomp\XmlWriter.cpp.txt); bodies in
     App/v8xml/TextXmlWriter.cpp.
   NOTE: split out of XmlSerializer.h (which keeps the dormant 2016
     TextXmlParser declaration) so writer users don't pull the 2016 parser
     decl that conflicts with v8xml/TextXmlParser.h.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8xml/XmlSerializer.cpp */

#pragma once

#include <ostream>
#include <map>

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>

#include "V8Xml/XmlElement.h"

namespace RBX
{
	class ContentProvider;
}

class RBXBaseClass XmlWriter : public boost::noncopyable {

protected:
	std::map<RBX::InstanceHandle, int> handles;
	typedef boost::unordered_map<std::string, RBX::InstanceHandle> IdValidationMap;
	IdValidationMap idValidationMap;
	std::ostream& stream;
	XmlWriter(std::ostream& stream);
public:
	virtual void serialize(const XmlElement* xmlNode) = 0;

	int getHandleIndex(RBX::InstanceHandle h) {
		int i;
		std::map<RBX::InstanceHandle, int>::iterator iter = handles.find(h);
		if (iter==handles.end()) {
			i = static_cast<int>(handles.size());	// TODO:  Should this be size_t getHandleIndex?
			handles[h] = i;
			return i;
		} else
			i = iter->second;
		return i;
	}

	bool isValidId(const std::string& id, const RBX::InstanceHandle& h) const
	{
		IdValidationMap::const_iterator itr = idValidationMap.find(id);
		return itr == idValidationMap.end() || (itr->second.getTarget() == h.getTarget());
	}

	void recordId(const std::string& id, const RBX::InstanceHandle& h)
	{
		RBXASSERT(isValidId(id, h));
		idValidationMap[id] = h;
	}
};

class TextXmlWriter : public XmlWriter
{
public:
	TextXmlWriter(std::ostream& stream)
		:XmlWriter(stream)
	{}
	// mac: 0x5BD818.
	void serialize(const XmlElement* xmlNode);
	// mac: 0x5BBB5C — XML-escaping writer (no CDATA path; the 2016
	// xmlEncodedWrite/xmlOrCDataEncodedWrite split is later). If len == 0,
	// strlen(text) is used.
	static void encodedWrite(std::ostream& stream, const char* text, unsigned long len);
protected:
	// mac: 0x5BD83E.
	void serialize(const XmlElement* xmlNode, int depth);
	// mac: 0x5BBCC4 — the extra attribute is the ContentId mimeType attr (NULL normally).
	void writeOpenTag(const XmlElement* element, int depth, const XmlAttribute* attr);
	// mac: 0x5BBF8A.
	void writeCloseTag(const XmlElement* element, int depth);
	// mac: 0x5BD8F6.
	void serializeNode(const XmlElement* xmlNode, int depth);
};
