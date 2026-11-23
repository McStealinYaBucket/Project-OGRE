/* v8xml/TextXmlParser.h -- the October 2013 XML text parser.
   Bodies are in Client/App/v8xml/XmlSerializer.cpp (its asserts cite that file: removeTag :291/:299,
   findNextToken :310, decodeString :186).

   Layout, from the 2013 binary (not the prior tree's guess of a vptr + deque + virtual dtor):
   - XmlParser ctor PLAYER 0x00628980: vptr 0x00b2b7dc, the streambuf at +0x04, a std::stack (VC9
     deque) at +0x08..+0x28. Its vftable has one slot, the pure parse(); no virtual destructor.
     RTTI lists boost::noncopyable as a base.
   - TextXmlParser ctor (inline, shell copy 0x00402c00): XmlParser(sb), vptr 0x00b2e040, then a
     std::map<std::string, std::string> at +0x28 (node colour at +0x45 = a 0x38-byte pair).
   - parse() returns the root through a hidden auto_ptr (SerializerV2::loadXML 0x006fcc00 releases it).
   - removeTag 0x00628550 and parseAttributes 0x00628d10 are members that never read this (ret 0xc /
     ret 4 with the hidden return pointer). */

#pragma once

#include <map>
#include <memory>
#include <stack>
#include <streambuf>
#include <string>

#include <boost/noncopyable.hpp>

class XmlElement;

class XmlParser : public boost::noncopyable
{
protected:
	std::streambuf* buffer;					// 0x04
	std::stack<XmlElement*> elements;		// 0x08
	XmlParser(std::streambuf* buffer);
public:
	virtual std::auto_ptr<XmlElement> parse() = 0;	// vtable+0x00
};

class TextXmlParser : public XmlParser
{
	std::map<std::string, std::string> legacyHashes;	// 0x28
public:
	TextXmlParser(std::streambuf* buffer)
		: XmlParser(buffer)
	{
	}
	virtual std::auto_ptr<XmlElement> parse();	// vtable+0x00, PLAYER 0x00628f70

private:
	void skipWhitespace();
	std::string readFirstTag();
	std::string readTag();
	std::string readText(bool decode);
	std::string removeTag(const std::string& contents, int& index);
	XmlElement* parseAttributes(const std::string& currentTag);
};

std::string decodeString(const std::string& text);
