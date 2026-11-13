/* App/v8xml/XmlSerializer.cpp -- October 2013 text XML parser and writer.
   2013 compiles TextXmlParser and TextXmlWriter in this one TU: their assert strings cite
   XmlSerializer.cpp (removeTag :291/:299) and their PLAYER bodies interleave (0x00627c50-
   0x00629630). The prior tree split them into v8xml/TextXmlParser.cpp and TextXmlWriter.cpp
   (headers kept: V8Xml/TextXmlParser.h, TextXmlWriter.h) and kept a dormant 2016 copy of this
   file out of the build; that copy is replaced by the 2013 bodies here. Definitions with a
   marker are in PLAYER address order; unmarked helpers come first.

   From TextXmlParser.cpp:
   App/v8xml/TextXmlParser.cpp — reconstructed for October 2013.
   Evidence: see TextXmlParser.h (EAs + assert paths + error strings).
   Error/assert strings are verbatim from the binary; DOM-building flow follows
   parse() 0x5BC6E8 (element stack via deque, ContentId-typed special-casing,
   self-closing pop, close-tag pop/root-finish).

   From TextXmlWriter.cpp:
   App/v8xml/TextXmlWriter.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
   (F:\2013Decompile\exports\decomp\XmlWriter.cpp.txt)
   - encodedWrite 0x5BBB5C: XML-escaping writer over (text, len); len == 0
   falls back to strlen. Escapes: > < " & ' to entities; bytes > 0x7E or
   < 0x20 except \r \n to &#%d; (notably \t IS escaped); else raw.
   NO CDATA path (the 2016 xmlEncodedWrite/xmlOrCDataEncodedWrite split
   and kCDATA_* constants are later additions — carried over in the dormant
   2016 XmlSerializer.cpp, not used here).
   - writeOpenTag 0x5BBCC4: depth tabs + '<' + tag + each attribute
   (' ' name '="' encoded(value) '"') + optional extra attr (the ContentId
   mimeType attr) + '>'. Never self-closes.
   - writeCloseTag 0x5BBF8A: depth tabs + '</' + tag + '>'.
   - serialize 0x5BD818/0x5BD83E: null node = no-op; serializeNode, then for
   each child '\n' + serialize(child, depth+1); children ? ('\n' +
   writeCloseTag(node, depth)) : writeCloseTag(node, 0).
   - serializeNode 0x5BD8F6: ContentId values get the mime-attr open tag when
   their Name is non-null (exact Name source TODO — likely the ContentId's
   name field; our getValue<const Name*> path leaves nullName for CONTENTID
   pairs, so the plain-tag branch is taken pending verification), then:
   xsi:nil attrs skip the body; empty id -> <null></null>; rbxasset:///http
   ids -> <url>encoded</url>; other ids -> StandardOut MESSAGE_WARNING
   "Not writing binary data" + <null></null>. Non-ContentId values:
   open tag + encoded(value.toString()).
   TODO(decompile): ContentId mime-Name source; exact "Not writing binary data"
   printf arity (binary passes one extra arg — FormatString remediation). */

#include "stdafx.h"
#include "v8xml/TextXmlParser.h"
#include "v8xml/XmlElement.h"
#include "Util/ContentId.h"
#include "util/standardout.h"
#include "reflection/Type.h"
#include <streambuf>
#include "V8Xml/TextXmlWriter.h"
#include "Util/Name.h"
#include <cstdio>
#include <cstring>

namespace {
	bool isWhitespace(char c)
	{
		return c == ' ' || c == '\t' || c == '\n' || c == '\r';
	}
	bool isNameChar(char c)
	{
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
			|| (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-'
			|| c == ':' || c == '/';
	}
}

XmlParser::XmlParser(std::streambuf* sb)  // 0x5BC666
	: streambuf(sb)
{
}

int TextXmlParser::peekChar()
{
	std::streambuf::int_type c = streambuf->sgetc();
	if (c == std::streambuf::traits_type::eof())
		return -1;
	return (unsigned char)c;
}

int TextXmlParser::getChar()
{
	std::streambuf::int_type c = streambuf->sbumpc();
	if (c == std::streambuf::traits_type::eof())
		return -1;
	return (unsigned char)c;
}

void TextXmlParser::ungetChar()
{
	streambuf->sungetc();
}

// ContentId-typed element value path (0x5BC6E8 inner dispatch). Grammar:
//   <ContentId ...>direct-text</ContentId> -> text DROPPED by the binary (link+pop;
//     TODO: confirm — no valid file seems to use it)
//   <ContentId ...><binary...>...</binary></ContentId> -> "Not reading binary data",
//     text skipped, null-Name value
//   <ContentId ...><hash>...</hash></ContentId> -> null-Name value
//   <ContentId ...><url>...</url></ContentId> -> ContentId(text, xsi:nil-name)
//   <ContentId ...><null>...</null></ContentId> -> null-Name value
//   anything else -> throw "Unknown tag '%s'."
// A closing tag is then consumed ("'%s' should be a closing tag" otherwise).
void TextXmlParser::readContentIdValue(XmlElement* element)
{
	std::string text;
	readText(text, false);
	if (!text.empty())
		return;  // binary drops direct text here (TODO: confirm no valid file uses it)
	const RBX::Name* nullName = &RBX::Name::getNullName();
	const XmlAttribute* xsiNil = element->findAttribute(*name_xsinil);
	if (xsiNil != NULL)
		xsiNil->getValue(nullName);
	std::string inner;
	readTag(inner);
	std::string innerName = inner;
	if (innerName.size() >= 2 && innerName[0] == '<')
		innerName = innerName.substr(1, innerName.size() - 2);
	if (innerName.compare(0, 6, "binary") == 0)
	{
		RBX::StandardOut::singleton()->print(RBX::MESSAGE_WARNING, "Not reading binary data");
		readText(text, false);
		element->setValue(&RBX::Name::getNullName());
	}
	else if (innerName == tag_hash->str)
	{
		readText(text, false);
		element->setValue(&RBX::Name::getNullName());
	}
	else if (innerName.compare(0, 3, "url") == 0)
	{
		readText(text, true);
		element->setValue(RBX::ContentId(text, *nullName));
	}
	else
	{
		if (innerName != tag_null->str)
			throw std::runtime_error(std::string("TextXmlParser::parse - Unknown tag '") + innerName + "'.");
		element->setValue(&RBX::Name::getNullName());
	}
	std::string close;
	readTag(close);
	if (!(close.size() >= 2 && close[0] == '<' && close[1] == '/'))
		throw std::runtime_error(std::string("TextXmlParser::parse - '") + close + "' should be a closing tag");
}

XmlWriter::XmlWriter(std::ostream& stream)
	: stream(stream)
{
}

void TextXmlWriter::writeCloseTag(const XmlElement* element, int depth)
{
	// mac: 0x5BBF8A.
	for (int i = 0; i < depth; ++i)
		stream << '\t';
	stream << "</" << element->getTag().c_str() << '>';
}

// mac: 0x5BAD68
// STUB: PLAYER 0x00627c50
// STUB: STUDIO 0x0074bb70
int TextXmlParser::skipWhitespace()
{
	while (true)
	{
		int c = peekChar();
		if (c == -1)
			return -1;
		if (!isWhitespace((char)c))
			return c;
		getChar();
	}
}

// mac: 0x5BADC4: BOM-tolerant first-tag read ("<...>" full text)
// STUB: PLAYER 0x00627c90
// STUB: STUDIO 0x0074bbb0
void TextXmlParser::readFirstTag(std::string& out)
{
	skipWhitespace();
	out.clear();
	for (int tries = 0; tries < 5; ++tries)
	{
		int c = getChar();
		if (c == -1)
			throw std::runtime_error("Expected '<' but got EOF in Xml stream");
		if (c != '<')
			continue;  // Byte-Order-Mark bytes
		out.push_back((char)c);
		while (true)
		{
			c = getChar();
			if (c == -1)
				throw std::runtime_error("Expected '>' but got EOF in Xml stream");
			out.push_back((char)c);
			if (c == '>')
				return;
		}
	}
	throw std::runtime_error("tag expected after Byte-Order-Mark");
}

// mac: 0x5BB0B2: full "<...>" text
// STUB: PLAYER 0x00627e20
// STUB: STUDIO 0x0074bd40
void TextXmlParser::readTag(std::string& out)
{
	skipWhitespace();
	int c = getChar();
	if (c == -1)
		throw std::runtime_error("EOF encountered while reading Tag start");
	if (c != '<')
		throw std::runtime_error("tag expected");
	out.clear();
	out.push_back((char)c);
	while (true)
	{
		c = getChar();
		if (c == -1)
			throw std::runtime_error("EOF encountered while reading Tag");
		out.push_back((char)c);
		if (c == '>')
			return;
	}
}

// mac: 0x5BB385
// STUB: PLAYER 0x006280a0
// STUB: STUDIO 0x0074bf30
void decodeString(std::string& out, const std::string& in)
{
	out.clear();
	for (size_t i = 0; i < in.size();)
	{
		char c = in[i];
		if (c != '&')
		{
			out.push_back(c);
			++i;
			continue;
		}
		size_t semi = in.find(';', i);
		if (semi == std::string::npos)
		{
			out.append(in, i, std::string::npos);
			break;
		}
		std::string entity = in.substr(i + 1, semi - i - 1);
		if (entity == "lt")
			out.push_back('<');
		else if (entity == "gt")
			out.push_back('>');
		else if (entity == "amp")
			out.push_back('&');
		else if (entity == "quot")
			out.push_back('"');
		else if (entity == "apos")
			out.push_back('\'');
		else if (entity == "nbsp")
			out.push_back(' ');
		else if (!entity.empty() && entity[0] == '#')
		{
			// TODO(decompile): verify numeric-ref tail of 0x5BB385 (base16? errors?)
			unsigned long code = strtoul(entity.c_str() + 1, NULL, 10);
			out.push_back(static_cast<char>(code & 0xFF));
		}
		else
			out.append(in, i, semi - i + 1);
		i = semi + 1;
	}
}

// mac: 0x5BBA28
// STUB: PLAYER 0x00628460
// STUB: STUDIO 0x0074c2f0
void TextXmlParser::readText(std::string& out, bool decode)
{
	skipWhitespace();
	std::string raw;
	while (true)
	{
		int c = peekChar();
		if (c == -1 || c == '<')
			break;
		raw.push_back((char)getChar());
	}
	if (decode && raw.find('&') != std::string::npos)
		decodeString(out, raw);
	else
		out = raw;
}

// mac: 0x5BC026
// STUB: PLAYER 0x00628550
// STUB: STUDIO 0x0074c3e0
std::string TextXmlParser::removeTag(const std::string& contents, int pos, int& index)
{
	RBXASSERT(contents[pos] == '<');  // XmlSerializer.cpp:291
	size_t i = (size_t)pos + 1;
	while (i < contents.size() && isNameChar(contents[i]))
		++i;
	size_t nameEnd = i;
	while (i < contents.size() && isWhitespace(contents[i]))
		++i;
	RBXASSERT((int)i > pos + 1);  // XmlSerializer.cpp:299 "index > start"
	index = (int)i;
	return contents.substr((size_t)pos + 1, nameEnd - (size_t)pos - 1);
}

// STUB: PLAYER 0x00628690
// STUB: STUDIO 0x0074c520
void TextXmlWriter::encodedWrite(std::ostream& stream, const char* text, unsigned long len)
{
	// mac: 0x5BBB5C: len == 0 -> strlen(text).
	if (len == 0)
	{
		if (text == NULL)
			return;
		len = (unsigned long)strlen(text);
	}
	for (unsigned long i = 0; i < len; ++i)
	{
		unsigned char c = static_cast<unsigned char>(text[i]);
		switch (c)
		{
		case '>':
			stream << "&gt;";
			break;
		case '<':
			stream << "&lt;";
			break;
		case '"':
			stream << "&quot;";
			break;
		case '&':
			stream << "&amp;";
			break;
		case '\'':
			stream << "&apos;";
			break;
		default:
			if (c > 0x7E || (c < 0x20 && c != '\r' && c != '\n'))
			{
				char encoded[16];
				sprintf(encoded, "&#%d;", c);
				stream << encoded;
			}
			else
			{
				stream << (char)c;
			}
			break;
		}
	}
}

// STUB: PLAYER 0x006287d0
// STUB: STUDIO 0x0074c6a0
void TextXmlWriter::writeOpenTag(const XmlElement* element, int depth, const XmlAttribute* attr)
{
	// mac: 0x5BBCC4.
	for (int i = 0; i < depth; ++i)
		stream << '\t';
	stream << '<' << element->getTag().c_str();
	for (const XmlAttribute* attribute = element->getFirstAttribute();
		attribute != NULL;
		attribute = element->getNextAttribute(attribute))
	{
		stream << ' ' << attribute->getTag().c_str() << "=\"";
		encodedWrite(stream, attribute->toString(this).c_str(), 0);
		stream << '"';
	}
	if (attr != NULL)
	{
		stream << ' ' << attr->getTag().c_str() << "=\"";
		encodedWrite(stream, attr->toString(this).c_str(), 0);
		stream << '"';
	}
	stream << '>';
}

// STUB: PLAYER 0x00628a60
// STUB: STUDIO 0x0074c8d0
void TextXmlWriter::serializeNode(const XmlElement* xmlNode, int depth)
{
	// mac: 0x5BD8F6.
	if (xmlNode->isValueType<RBX::ContentId>())
	{
		RBX::ContentId contentId;
		xmlNode->getValue(contentId);
		// Mime-attr branch: the binary compares a Name carried by the value
		// against nullName (TODO: exact source — likely the ContentId name
		// field; getValue<const Name*> leaves nullName for CONTENTID pairs).
		const RBX::Name* mimeName = &RBX::Name::getNullName();
		xmlNode->getValue(mimeName);
		if (mimeName == &RBX::Name::getNullName())
		{
			writeOpenTag(xmlNode, depth, NULL);
		}
		else
		{
			XmlAttribute mimeAttr(*tag_mimeType, mimeName->c_str());
			writeOpenTag(xmlNode, depth, &mimeAttr);
		}
		if (xmlNode->findAttribute(*name_xsinil) == NULL)
		{
			const std::string& id = contentId.toString();
			if (!id.empty())
			{
				if (id.compare(0, 11, "rbxasset://") == 0
					|| id.compare(0, 4, "http") == 0)
				{
					stream << "<url>";
					encodedWrite(stream, id.c_str(), (unsigned long)id.size());
					stream << "</url>";
				}
				else
				{
					RBX::StandardOut::singleton()->printf(RBX::MESSAGE_WARNING,
						"Not writing binary data");
					stream << "<null></null>";
				}
			}
			else
			{
				stream << "<null></null>";
			}
		}
	}
	else
	{
		writeOpenTag(xmlNode, depth, NULL);
		encodedWrite(stream, xmlNode->toString(this).c_str(), 0);
	}
}

// STUB: PLAYER 0x00628c90
// STUB: STUDIO 0x0074cb00
void TextXmlWriter::serialize(const XmlElement* xmlNode, int depth)
{
	// mac: 0x5BD83E.
	if (xmlNode == NULL)
		return;
	serializeNode(xmlNode, depth);
	if (xmlNode->firstChild() != NULL)
	{
		for (const XmlElement* child = xmlNode->firstChild();
			child != NULL;
			child = xmlNode->nextChild(child))
		{
			stream << '\n';
			serialize(child, depth + 1);
		}
		stream << '\n';
		writeCloseTag(xmlNode, depth);
	}
	else
	{
		writeCloseTag(xmlNode, 0);
	}
}

// mac: 0x5BC158
// STUB: PLAYER 0x00628d10
// STUB: STUDIO 0x0074cb80
XmlElement* TextXmlParser::parseAttributes(const std::string& tag)
{
	int index = 0;
	std::string name = removeTag(tag, 0, index);
	XmlElement* element = new XmlElement(*RBX::Name::declare(name.c_str()));
	size_t i = (size_t)index;
	while (i < tag.size())
	{
		char c = tag[i];
		if (c == 0 || c == '>')
			break;
		if (isWhitespace(c))
		{
			++i;
			continue;
		}
		size_t eq = tag.find('=', i);
		if (eq == std::string::npos)
			throw std::runtime_error("Unable to parse XML attributes. '=' not found");
		std::string attrName = tag.substr(i, eq - i);
		size_t q1 = tag.find('"', eq + 1);
		if (q1 == std::string::npos)
			throw std::runtime_error("Unable to parse XML attributes. '\"' not found");
		size_t q2 = tag.find('"', q1 + 1);
		if (q2 == std::string::npos)
			throw std::runtime_error("Unable to parse XML attributes. '\"' not found");
		std::string value = tag.substr(q1 + 1, q2 - q1 - 1);
		if (value.find('&') != std::string::npos)
		{
			std::string decoded;
			decodeString(decoded, value);
			value.swap(decoded);
		}
		i = q2 + 1;
		element->addAttribute(*RBX::Name::declare(attrName.c_str()), value);
	}
	return element;
}

// mac: 0x5BC6E8: stream -> XmlElement DOM. Element stack in the XmlParser deque;
// ContentId-typed elements take the mimeType/xsinil/hash/url value path,
// everything else takes readText(decode) + setValue. Self-closing "/>" pops.
// STUB: PLAYER 0x00628f70
// STUB: STUDIO 0x0074cde0
XmlElement* TextXmlParser::parse()
{
	if (peekChar() == -1)
		throw std::runtime_error("TextXmlParser::parse empty file");

	std::string tag;
	readFirstTag(tag);
	if (tag.compare(0, 2, "<?") != 0)
	{
		// First tag already read (no prolog) — process it, then readTag onwards.
	}
	else
		readTag(tag);

	XmlElement* root = NULL;
	while (true)
	{
		if (tag.size() >= 2 && tag[0] == '<' && tag[1] == '/')
		{
			if (elements.empty())
				throw std::runtime_error(std::string("TextXmlParser::parse - Got close tag ") + tag + " without open tag.");
			elements.pop_back();
			if (elements.empty())
				break;
		}
		else
		{
			XmlElement* element = parseAttributes(tag);
			if (!elements.empty())
				elements.back()->addChild(element);
			else if (!root)
				root = element;
			elements.push_back(element);

			if (element->getTag() != *RBX::Reflection::Type::getSingleton<RBX::ContentId>().tag)
			{
				std::string text;
				readText(text, true);
				element->setValue(text);
			}
			else
			{
				readContentIdValue(element);
			}

			if (tag.size() >= 2 && tag[tag.size() - 2] == '/' && tag[tag.size() - 1] == '>')
				elements.pop_back();
		}
		readTag(tag);
	}
	return root;
}

// FUNCTION: PLAYER 0x00629630
// FUNCTION: STUDIO 0x0074d4a0
void TextXmlWriter::serialize(const XmlElement* xmlNode)
{
	// mac: 0x5BD818.
	serialize(xmlNode, 0);
}
