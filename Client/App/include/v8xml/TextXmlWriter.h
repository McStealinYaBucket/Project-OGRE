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

#include "V8Xml/XmlElement.h"

namespace RBX
{
	class ContentProvider;
}

// 2013 layout, from the out-of-line ctor PLAYER 0x00628a20: the handle map at +0x04 and the
// stream at +0x10, nothing else (2016's idValidationMap/recordId did not exist yet). novtable:
// the ctor stores no vptr; TextXmlWriter's vftable 0x00b169c0 has the one serialize slot.
class RBXBaseClass XmlWriter : public boost::noncopyable {

protected:
	std::map<RBX::InstanceHandle, int> handles;	// 0x04
	std::ostream& stream;						// 0x10
	XmlWriter(std::ostream& stream);
public:
	virtual void serialize(const XmlElement* xmlNode) = 0;

	// emitted in XmlElement.obj for XmlNameValuePair::toString
	// FUNCTION: PLAYER 0x00614950
	// FUNCTION: STUDIO 0x006db460
	int getHandleIndex(RBX::InstanceHandle h) {
		int i;
		std::map<RBX::InstanceHandle, int>::iterator iter = handles.find(h);
		if (iter==handles.end()) {
			i = static_cast<int>(handles.size());
			handles[h] = i;
			return i;
		} else
			i = iter->second;
		return i;
	}
};

// std::map<InstanceHandle, int> bodies XmlElement.obj emits for getHandleIndex (no oracle names;
// byte-identical to PLAYER under tools/probe_compare.py). STUDIO copies are byte-identical too.
// clang-format off

// TEMPLATE: PLAYER 0x00613d80
// TEMPLATE: STUDIO 0x006da8b0
// ?_Lbound@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@HU?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@H@std@@@4@$0A@@std@@@std@@IBEPAU_Node@?$_Tree_nod@V?$_Tmap_traits@VInstanceHandle@RBX@@HU?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@H@std@@@4@$0A@@std@@@2@ABVInstanceHandle@RBX@@@Z

// TEMPLATE: PLAYER 0x00614390
// TEMPLATE: STUDIO 0x006daea0
// ?_Buynode@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@HU?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@H@std@@@4@$0A@@std@@@std@@IAEPAU_Node@?$_Tree_nod@V?$_Tmap_traits@VInstanceHandle@RBX@@HU?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@H@std@@@4@$0A@@std@@@2@PAU342@00ABU?$pair@$$CBVInstanceHandle@RBX@@H@2@D@Z

// TEMPLATE: PLAYER 0x006143f0
// TEMPLATE: STUDIO 0x006daf00
// ?_Insert@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@HU?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@H@std@@@4@$0A@@std@@@std@@IAE?AViterator@12@_NPAU_Node@?$_Tree_nod@V?$_Tmap_traits@VInstanceHandle@RBX@@HU?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@H@std@@@4@$0A@@std@@@2@ABU?$pair@$$CBVInstanceHandle@RBX@@H@2@@Z

// TEMPLATE: PLAYER 0x006145f0
// TEMPLATE: STUDIO 0x006db100
// ?insert@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@HU?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@H@std@@@4@$0A@@std@@@std@@QAE?AU?$pair@Viterator@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@HU?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@H@std@@@4@$0A@@std@@@std@@_N@2@ABU?$pair@$$CBVInstanceHandle@RBX@@H@2@@Z

// TEMPLATE: PLAYER 0x006146b0
// TEMPLATE: STUDIO 0x006db1c0
// ?insert@?$_Tree@V?$_Tmap_traits@VInstanceHandle@RBX@@HU?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@H@std@@@4@$0A@@std@@@std@@QAE?AViterator@12@Vconst_iterator@12@ABU?$pair@$$CBVInstanceHandle@RBX@@H@2@@Z

// TEMPLATE: PLAYER 0x00614870
// TEMPLATE: STUDIO 0x006db380
// ??A?$map@VInstanceHandle@RBX@@HU?$less@VInstanceHandle@RBX@@@std@@V?$allocator@U?$pair@$$CBVInstanceHandle@RBX@@H@std@@@4@@std@@QAEAAHABVInstanceHandle@RBX@@@Z
// clang-format on

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
	// Kept inline because writeOpenTag 0x006287d0 destroys the toString temporary before the closing quote
	static void encodedWrite(std::ostream& stream, const std::string& text) {
		encodedWrite(stream, text.c_str(), text.size());
	}
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
