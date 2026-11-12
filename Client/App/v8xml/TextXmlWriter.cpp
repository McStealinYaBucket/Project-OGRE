/* App/v8xml/TextXmlWriter.cpp — reconstructed for October 2013.
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
     printf arity (binary passes one extra arg — FormatString remediation).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8xml/XmlSerializer.cpp */

#include "stdafx.h"

#include "V8Xml/TextXmlWriter.h"
#include "V8Xml/XmlElement.h"
#include "Util/ContentId.h"
#include "Util/StandardOut.h"
#include "Util/Name.h"

#include <cstdio>
#include <cstring>

XmlWriter::XmlWriter(std::ostream& stream)
	: stream(stream)
{
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

void TextXmlWriter::writeCloseTag(const XmlElement* element, int depth)
{
	// mac: 0x5BBF8A.
	for (int i = 0; i < depth; ++i)
		stream << '\t';
	stream << "</" << element->getTag().c_str() << '>';
}

// FUNCTION: PLAYER 0x00629630
// FUNCTION: STUDIO 0x0074d4a0
void TextXmlWriter::serialize(const XmlElement* xmlNode)
{
	// mac: 0x5BD818.
	serialize(xmlNode, 0);
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
