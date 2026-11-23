/* App/v8xml/XmlSerializer.cpp -- the October 2013 text XML reader (TextXmlParser) and writer
   (TextXmlWriter), one translation unit as in 2013: the asserts of both cite XmlSerializer.cpp and
   their PLAYER bodies interleave (0x00627b80-0x00629630). Bodies are written from the PLAYER bytes.
   __LINE__ is pinned by layout: decodeString's assert is line 186, removeTag's 291 and 299,
   findNextToken's 310. VC9 does not emit this .obj in source order (findNextToken 0x00627f90 comes
   before decodeString 0x006280a0); the definitions follow the asserts. */

#include "stdafx.h"
#include "v8xml/TextXmlParser.h"
#include "v8xml/XmlElement.h"
#include "Util/ContentId.h"
#include "util/standardout.h"
#include "reflection/Type.h"
#include "V8Xml/TextXmlWriter.h"
#include "Util/Name.h"
#include "RbxFormat.h"
#include <streambuf>
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <cstdlib>

// 0x00d30dd0 (.data, 256 bytes, statically initialised): tab, LF, FF, CR and space. Indexed with a
// signed char (movsx) by skipWhitespace, findNextToken and removeTag.
// GLOBAL: PLAYER 0x00d30dd0
// GLOBAL: STUDIO 0x00ebd608
static bool whitespace[256] = {
	0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

// FUNCTION: PLAYER 0x00627c50
// FUNCTION: STUDIO 0x0074bb70
void TextXmlParser::skipWhitespace()
{
	int c = buffer->sgetc();
	while (c != EOF)
	{
		if (!whitespace[(char)c])
			break;
		buffer->sbumpc();
		c = buffer->sgetc();
	}
}

// FUNCTION: PLAYER 0x00627c90
// FUNCTION: STUDIO 0x0074bbb0
std::string TextXmlParser::readFirstTag()
{
	skipWhitespace();
	// up to four Byte-Order-Mark bytes may come before the first '<'
	int skipped = 0;
	char c;
	do
	{
		if (buffer->sgetc() == EOF)
			throw std::runtime_error("Expected '<' but got EOF in Xml stream");
		if (skipped++ > 4)
		{
			std::string what = "tag expected after Byte-Order-Mark";
			throw std::runtime_error(what);
		}
		c = (char)buffer->sbumpc();
	} while (c != '<');
	std::string result;
	result += '<';
	do
	{
		if (buffer->sgetc() == EOF)
			throw std::runtime_error("Expected '>' but got EOF in Xml stream");
		c = (char)buffer->sbumpc();
		result += c;
	} while (c != '>');
	return result;
}

// FUNCTION: PLAYER 0x00627e20
// FUNCTION: STUDIO 0x0074bd40
std::string TextXmlParser::readTag()
{
	skipWhitespace();
	if (buffer->sgetc() == EOF)
		throw std::runtime_error("EOF encountered while reading Tag start");
	if ((char)buffer->sbumpc() != '<')
		throw std::runtime_error("tag expected");
	std::string result;
	result += '<';
	char c;
	do
	{
		if (buffer->sgetc() == EOF)
			throw std::runtime_error("EOF encountered while reading Tag");
		c = (char)buffer->sbumpc();
		result += c;
	} while (c != '>');
	return result;
}








































// FUNCTION: PLAYER 0x006280a0
// FUNCTION: STUDIO 0x0074bf30
std::string decodeString(const std::string& text)
{
	std::string result;
	size_t i = 0;
	while (i < text.size())
	{
		char c = text[i++];
		if (c == '&')
		{
			std::string entity;	// the text between '&' and ';'
			while (i < text.size())
			{
				c = text[i++];
				if (c != ';')
					entity += c;
				else
					break;
			}
			if (entity == "lt")
				result += '<';
			else if (entity == "gt")
				result += '>';
			else if (entity == "amp")
				result += '&';
			else if (entity == "quot")
				result += '"';
			else if (entity == "apos")
				result += '\'';
			else if (entity == "nbsp")
				result += ' ';
			else if (entity[0] == '#')
			{
				if (entity.size() < 2)
					throw std::runtime_error("bad XML. No character code following #");
				if (entity[1] == 'x')
					throw std::runtime_error("Unable to parse hexidecimal character code");
				result += (char)atoi(entity.substr(1).c_str());
			}
			else
			{
				RBXASSERT(false);
				result += "&" + entity + ";";
			}
		}
		else
			result += c;
	}
	return result;
}











































































// FUNCTION: PLAYER 0x00628460
// FUNCTION: STUDIO 0x0074c2f0
std::string TextXmlParser::readText(bool decode)
{
	skipWhitespace();
	std::string text;
	int c = buffer->sgetc();
	while (c != EOF && (char)c != '<')
	{
		text += (char)buffer->sbumpc();
		c = buffer->sgetc();
	}
	if (decode && text.find('&') != std::string::npos)
		return decodeString(text);
	return text;
}

// FUNCTION: PLAYER 0x00628550
// FUNCTION: STUDIO 0x0074c3e0
std::string TextXmlParser::removeTag(const std::string& contents, int& index)
{
	RBXASSERT(contents[0] == '<');
	int start = 1;
	while (whitespace[contents[start]] && start < (int)contents.size())
		++start;
	index = start;
	while (!whitespace[contents[index]] && contents[index] != '>' && index < (int)contents.size())
		++index;

	RBXASSERT(index > start);
	return contents.substr(start, index - start);
}

// FUNCTION: PLAYER 0x00627f90
// FUNCTION: STUDIO 0x0074beb0
static bool findNextToken(const std::string& contents, int& index)
{
	const char* c = contents.c_str() + index;
	while (true)
	{
		RBXASSERT(*c);
		if (*c == '>')
			return false;
		if (*c == 0)
			return false;
		if (!whitespace[*c])
			return true;
		++index;
		++c;
	}
}

// FUNCTION: PLAYER 0x00628690
// FUNCTION: STUDIO 0x0074c520
void TextXmlWriter::encodedWrite(std::ostream& stream, const char* text, unsigned long len)
{
	if (len == 0)
		len = (unsigned long)strlen(text);
	for (unsigned long i = 0; i < len; ++i)
	{
		char c = *text++;
		if (c == '<')
			stream << "&lt;";
		else if (c == '>')
			stream << "&gt;";
		else if (c == '&')
			stream << "&amp;";
		else if (c == '"')
			stream << "&quot;";
		else if (c == '\'')
			stream << "&apos;";
		else if (((unsigned char)c < 0x20 && c != '\n' && c != '\r') || (unsigned char)c > 0x7e)
		{
			char buffer[8];
			sprintf(buffer, "&#%d;", (unsigned char)c);
			stream << buffer;
		}
		else
			stream << c;
	}
}

// FUNCTION: PLAYER 0x00628770
// FUNCTION: STUDIO 0x0074c600
void TextXmlWriter::writeCloseTag(const XmlElement* element, int depth)
{
	for (int i = 0; i < depth; ++i)
		stream << '\t';
	stream << "</" << element->getTag().str << '>';
}

// FUNCTION: PLAYER 0x006287d0
// FUNCTION: STUDIO 0x0074c6a0
void TextXmlWriter::writeOpenTag(const XmlElement* element, int depth, const XmlAttribute* extra)
{
	for (int i = 0; i < depth; ++i)
		stream << '\t';
	stream << '<' << element->getTag().str;
	for (const XmlAttribute* attribute = element->getFirstAttribute(); attribute != NULL; attribute = element->getNextAttribute(attribute))
	{
		stream << ' ' << attribute->getTag().str << "=\"";
		encodedWrite(stream, attribute->toString(this));
		stream << '"';
	}
	if (extra != NULL)
	{
		stream << ' ' << extra->getTag().str << "=\"";
		encodedWrite(stream, extra->toString(this));
		stream << '"';
	}
	stream << '>';
}

// FUNCTION: PLAYER 0x00628980
// FUNCTION: STUDIO 0x0074c850
XmlParser::XmlParser(std::streambuf* sb)
	: buffer(sb)
{
}

// FUNCTION: PLAYER 0x00628a20
// FUNCTION: STUDIO 0x0074c890
XmlWriter::XmlWriter(std::ostream& stream)
	: stream(stream)
{
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

// FUNCTION: PLAYER 0x00628c90
// FUNCTION: STUDIO 0x0074cb00
void TextXmlWriter::serialize(const XmlElement* xmlNode, int depth)
{
	if (xmlNode == NULL)
		return;
	serializeNode(xmlNode, depth);
	if (const XmlElement* child = xmlNode->firstChild())
	{
		do
		{
			stream << '\n';
			serialize(child, depth + 1);
			child = xmlNode->nextChild(child);
		} while (child != NULL);
		stream << '\n';
		writeCloseTag(xmlNode, depth);
	}
	else
		writeCloseTag(xmlNode, 0);
}

// FUNCTION: PLAYER 0x00628d10
// FUNCTION: STUDIO 0x0074cb80
XmlElement* TextXmlParser::parseAttributes(const std::string& currentTag)
{
	int index = 0;
	std::string tagName = removeTag(currentTag, index);
	XmlElement* element = new XmlElement(*RBX::Name::lookup(tagName));
	while (findNextToken(currentTag, index))
	{
		const size_t equals = currentTag.find('=', index);
		const std::string name(currentTag.substr(index, equals - index));
		if (equals == std::string::npos)
			throw std::runtime_error("Unable to parse XML attributes. '=' not found");
		const int openQuote = equals + 1;
		const int closeQuote = currentTag.find('"', openQuote + 1);
		std::string value(currentTag.substr(openQuote + 1, closeQuote - openQuote - 1));
		if (value.find('&') != std::string::npos)
			value = decodeString(value);
		index = closeQuote + 1;
		element->addAttribute(*RBX::Name::lookup(name), value);
	}
	return element;
}

static inline bool isClosingTag(const char* tag)
{
	return tag[0] == '<' && tag[1] == '/';
}

static inline bool isSelfClosingTag(const std::string& tag)
{
	const size_t length = tag.size();
	if (length < 2)
		return false;
	const char* text = tag.c_str();
	return text[length - 2] == '/' && text[length - 1] == '>';
}

// STUB: PLAYER 0x00628f70
// STUB: STUDIO 0x0074cde0
std::auto_ptr<XmlElement> TextXmlParser::parse()
{
	if (buffer->sgetc() == EOF)
		throw std::runtime_error("TextXmlParser::parse empty file");

	bool first = true;
	while (true)
	{
		std::string tag;
		if (first)
		{
			first = false;
			tag = readFirstTag();
			if (tag.substr(0, 2) == "<?")
				continue;	// the <?xml ...?> prolog
		}
		else
			tag = readTag();

		XmlElement* parent = elements.empty() ? NULL : elements.top();
		if (isClosingTag(tag.c_str()))
		{
			if (parent == NULL)
				throw RBX::runtime_error("TextXmlParser::parse - Got close tag %s without open tag.", tag.c_str());
			elements.pop();
			if (elements.empty())
				return std::auto_ptr<XmlElement>(parent);
		}
		else
		{
			XmlElement* element = parseAttributes(tag);
			elements.push(element);
			if (element->getTag() == *RBX::Reflection::Type::getSingleton<RBX::ContentId>().tag)
			{
				const XmlAttribute* nilAttribute = element->findAttribute(*name_xsinil);
				bool isNil;
				if (nilAttribute != NULL && nilAttribute->getValue(isNil) && isNil)
				{
					// xsi:nil: no value
				}
				else if (readText(false) != "")
				{
					// a bare value inside a ContentId element is dropped
				}
				else
				{
					const RBX::Name* mimeType;
					const XmlAttribute* mimeAttribute = element->findAttribute(*tag_mimeType);
					if (mimeAttribute != NULL)
						mimeAttribute->getValue(mimeType);
					else
						mimeType = &RBX::Name::getNullName();

					std::string childTag = readTag();
					std::string childName = childTag.substr(1, childTag.size() - 2);
					if (childName.compare(0, 6, "binary") == 0)
					{
						RBX::StandardOut::singleton()->printf(RBX::MESSAGE_WARNING, "Not reading binary data");
						readText(false);
						element->setValue(RBX::ContentId());
					}
					else if (*tag_hash == childName)
					{
						readText(false);
						element->setValue(RBX::ContentId());
					}
					else if (childName.compare(0, 3, "url") == 0)
						element->setValue(RBX::ContentId(readText(true).c_str(), *mimeType));
					else if (*tag_null == childName)
						element->setValue(RBX::ContentId());
					else
						throw RBX::runtime_error("TextXmlParser::parse - Unknown tag '%s'.", childName.substr(0, 32).c_str());

					std::string closeTag = readTag();
					if (!isClosingTag(closeTag.c_str()))
						throw RBX::runtime_error("TextXmlParser::parse - '%s' should be a closing tag", closeTag.substr(0, 32).c_str());
				}
			}
			else
				element->setValue(readText(true));

			if (parent != NULL)
				parent->addChild(element);

			if (isSelfClosingTag(tag))
				elements.pop();	// <tag ... />
		}
	}
}

// FUNCTION: PLAYER 0x00629630
// FUNCTION: STUDIO 0x0074d4a0
void TextXmlWriter::serialize(const XmlElement* xmlNode)
{
	serialize(xmlNode, 0);
}
