#ifndef V8XML_XMLSERIALIZER_H
#define V8XML_XMLSERIALIZER_H

#include <vector>
#include <stack>
#include <ostream>

#include <boost/unordered_map.hpp>

#include "V8Xml/XmlElement.h"

#include "V8Xml/TextXmlWriter.h"

// NOTE: XmlWriter + TextXmlWriter now live in TextXmlWriter.h (2013 port).
// The TextXmlParser declaration below is the dormant 2016 template — the real
// 2013 parser is v8xml/TextXmlParser.h (do NOT include both in one TU).

class XmlParser : public boost::noncopyable {

protected:
	std::streambuf* buffer;
	XmlParser(std::streambuf* buffer);
	std::stack<XmlElement*> elements;		// stack
public:
	virtual std::auto_ptr<XmlElement> parse() = 0;
};

class TextXmlParser : public XmlParser {

	std::map<std::string, std::string> legacyHashes;		// workaround for a bug in the MD5 hasher
public:
	TextXmlParser(std::streambuf* buffer)
		:XmlParser(buffer)
	{}
	std::auto_ptr<XmlElement> parse();

private:
	void skipWhitespace(); 

	std::string readTag();
	std::string readFirstTag();
	std::string readText(bool decode);

	// attribute handling
	std::string removeTag(const std::string& contents, int &index);
	std::string findNextToken(const std::string& contents, int &index);
	std::string findText(const std::string& attribute);
	XmlElement* parseAttributes(const std::string& currentTag);
};



#endif
