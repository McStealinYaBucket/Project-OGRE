/* v8xml/TextXmlParser.h — October 2013 XML text parser (DOM builder).
   Evidence: symbolled Studio 0.126 decompilation
     (F:\2013Decompile\exports\decomp\BinderParser.cpp.txt)
      - XmlParser::XmlParser(streambuf*) 0x5BC666: vftable + streambuf + XmlElement* deque.
      - skipWhitespace 0x5BAD68, readFirstTag 0x5BADC4 (BOM-tolerant, "Expected '<'
        but got EOF in Xml stream" / "tag expected after Byte-Order-Mark" /
        "Expected '>' but got EOF in Xml stream"), readTag 0x5BB0B2 ("tag expected",
        "EOF encountered while reading Tag start/Tag"), readText(bool) 0x5BBA28
        (&-decode via decodeString), removeTag 0x5BC026 (asserts XmlSerializer.cpp
        :291 contents[0]=='<' and :299 index>start), parseAttributes 0x5BC158
        ("Unable to parse XML attributes. '=' not found"), parse 0x5BC6E8
        ("TextXmlParser::parse empty file", "Got close tag %s without open tag",
        "Not reading binary data"), decodeString 0x5BB385 (free function;
        lt/gt/amp/quot/apos/nbsp entities + numeric refs).
      - Lives in Client/App/v8xml/XmlSerializer.cpp per assert paths.
   Internal (static-vs-member) conventions are reconstructed for behavior; the
   observable contract (error strings, DOM output, asserts) matches the binary.
   TODO(decompile): byte_FB8698 vs byte_9FC52F table addresses per function (both
     ws/name scanning observed; port uses explicit isWhitespace/isNameChar helpers —
     behaviorally identical on valid input; a strict single shared table as literally
     decompiled would consume leading text in readText, impossible for a working
     parser — verify table references in IDA).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8xml/XmlSerializer.cpp */

#pragma once

#include <deque>
#include <iosfwd>
#include <string>

class XmlElement;

class XmlParser
{
public:
	explicit XmlParser(std::streambuf* sb);  // 0x5BC666
	virtual ~XmlParser() {}

protected:
	std::streambuf* streambuf;				// +4 (underflow=vtable+36, uflow=+40)
	std::deque<XmlElement*> elements;		// +8
};

class TextXmlParser : public XmlParser
{
public:
	explicit TextXmlParser(std::streambuf* sb) : XmlParser(sb) {}

	XmlElement* parse();  // 0x5BC6E8: stream -> XmlElement DOM root (throws on error)

private:
	// -1 on EOF, else the char (peeked for skip, consumed elsewhere)
	int peekChar();
	int getChar();
	void ungetChar();

	int skipWhitespace();						// 0x5BAD68
	void readFirstTag(std::string& out);		// 0x5BADC4
	void readTag(std::string& out);				// 0x5BB0B2: full "<...>" text
	void readText(std::string& out, bool decode);	// 0x5BBA28
	// mac: 0x5BC026: contents[pos] must be '<'; returns tag name, sets index past name+ws
	std::string removeTag(const std::string& contents, int pos, int& index);
	XmlElement* parseAttributes(const std::string& tag);	// 0x5BC158
	void readContentIdValue(XmlElement* element);
};

// mac: 0x5BB385: XML entity decoder (&lt; &gt; &amp; &quot; &apos; &nbsp; &#NNN;).
void decodeString(std::string& out, const std::string& in);
