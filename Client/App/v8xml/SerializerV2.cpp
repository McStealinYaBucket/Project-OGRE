/* App/v8xml/SerializerV2.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
     (F:\2013Decompile\exports\decomp\SerializerV2_full.cpp.txt 16/16 fns,
      BinderParser.cpp.txt binder EAs)
      - newRootElement() 0x5AF136 -> newRootElement("").
      - newRootElement(string const&) 0x5AF1DA: static xmlns:xmime Name; root
        tag_roblox + attrs xmlns:xmime="http://www.w3.org/2005/05/xmlmime" (PE-verified),
        xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" (literal in decompile),
        xsi:noNamespaceSchemaLocation="http://www.roblox.com/roblox.xsd" (PE-verified),
        version=4; optional assettype=type; External children with IDREF null+nil.
      - loadInstance 0x5AEB9E: class attr -> factory create -> read(); unknown class
        -> StandardOut 'Unknown object class "%s" while reading XML'.
      - loadInstances 0x5AEE84: root must be tag_roblox + version attr >= 4 else
        G3D::Log::println "Version 1.0 - 3.0 model files not supported..."; Item loop
        via loadInstance; binder.resolveRefs().
      - loadInstancesFromText 0x5AECFC: ArchiveBinder + loadInstances + resolveRefs.
      - loadInstancesFromMemory 0x5AF018: MemoryBinder + loadInstances.
      - load(XmlElement*, DataModel*) 0x5ADDDA: readChildren + ArchiveBinder resolveRefs.
      - loadXML 0x5AE83E: parse -> tag_roblox check ("ill-formed XML. No Roblox tag")
        -> version attr ("no version number", "schemaVersionLoading<4") -> load(root)
        -> DataModel vtable+140 call (TODO: identify method) -> cleanup.
      - ArchiveBinder::processID 0x5B10C4 (Super + idMap/setXmlId, asserts foundString
        SerializerV2.cpp:116); processIDREF 0x5B122C; resolveRefs 0x5AF506.
      - MemoryBinder::processID 0x5B0332 (InstanceHandle remap at +4); processIDREF
        0x5B0448; resolveRefs 0x5B0232.
      - load(istream&) 0x5ADF6C is a compound-file reader (header magic + version +
        contentPos + 4 reads; error strings "can't read header/old fileFormatVersion/
        fileFormatVersion!=1/can't read contentPos/error reading compound file (1-4)").
        Ported below (binary/XML dispatch + old-format compound loop).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8xml/SerializerV2.cpp */

#include "stdafx.h"

#include "V8Xml/SerializerV2.h"
#include "V8Xml/TextXmlParser.h"
#include "V8Xml/SerializerBinary.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/ContentProvider.h"
#include "rbx/Debug.h"
#include "util/standardout.h"

#include <cstring>
#include <map>
#include <sstream>

using namespace RBX;

// An implementation of IReferenceBinder
// STUB: PLAYER 0x006fb300
// STUB: STUDIO 0x00749890
bool ArchiveBinder::processID(const XmlNameValuePair* valueID, RBX::Reflection::DescribedBase* source)
{
	if (Super::processID(valueID, source))
		return true;
	std::string s;
	bool foundString = valueID->getValue(s);
	RBXASSERT(foundString);  // SerializerV2.cpp:116
	idMap[s].linkTo(shared_from(source));
	return true;
}

// STUB: PLAYER 0x006fb3f0
// STUB: STUDIO 0x00749980
bool ArchiveBinder::processIDREF(const XmlNameValuePair* valueIDREF, RBX::Reflection::DescribedBase* propertyOwner, const RBX::IIDREF* idref)
{
	// TODO(decompile): verify against 0x5B122C (follows the MergeBinder shape).
	return MergeBinder::processIDREF(valueIDREF, propertyOwner, idref);
}

// STUB: PLAYER 0x006fadd0
// STUB: STUDIO 0x007494e0
bool ArchiveBinder::resolveRefs()
{
	// TODO(decompile): verify against 0x5AF506 (MergeBinder drain + idMap clear?).
	bool ok = MergeBinder::resolveRefs();
	idMap.clear();
	return ok;
}

void MemoryBinder::announceID(const XmlNameValuePair* valueID, RBX::Reflection::DescribedBase* source)
{
	processID(valueID, source);
}

void MemoryBinder::announceIDREF(const XmlNameValuePair* valueIDREF, RBX::Reflection::DescribedBase* propertyOwner, const RBX::IIDREF* idref)
{
	processIDREF(valueIDREF, propertyOwner, idref);
}

// STUB: PLAYER 0x006fc420
// STUB: STUDIO 0x0074a9b0
bool MemoryBinder::resolveRefs()
{
	// TODO(decompile): verify against 0x5B0232.
	for (std::vector<IDREFItem>::iterator iter = deferredIDREFItems.begin(); iter != deferredIDREFItems.end(); ++iter)
		assign(iter->idref, iter->propertyOwner, iter->value);
	deferredIDREFItems.clear();
	return true;
}

// STUB: PLAYER 0x006fc500
// STUB: STUDIO 0x0074aa90
bool MemoryBinder::processID(const XmlNameValuePair* valueID, RBX::Reflection::DescribedBase* source)
{
	RBX::InstanceHandle h;
	if (valueID->getValue(h))
	{
		idMap[h].linkTo(shared_from(source));
		return true;
	}
	else if (valueID->isValueEqual(&value_IDREF_nil))
		return true;
	return false;
}

// STUB: PLAYER 0x006fc720
// STUB: STUDIO 0x0074acb0
bool MemoryBinder::processIDREF(const XmlNameValuePair* valueIDREF, RBX::Reflection::DescribedBase* propertyOwner, const RBX::IIDREF* idref)
{
	// TODO(decompile): verify against 0x5B0448 (follows the MergeBinder shape).
	RBX::InstanceHandle value;
	if (valueIDREF->getValue(value))
	{
		if (!value.empty())
			assign(idref, propertyOwner, value);
		else
		{
			IDREFItem item = {idref, propertyOwner, value};
			deferredIDREFItems.push_back(item);
		}
		return true;
	}
	else if (valueIDREF->isValueEqual(&value_IDREF_nil))
		return true;
	return false;
}

// FUNCTION: PLAYER 0x006fd510
// FUNCTION: STUDIO 0x0074baa0
XmlElement* SerializerV2::newRootElement()  // 0x5AF136
{
	return newRootElement("");
}

// STUB: PLAYER 0x006fcec0
// STUB: STUDIO 0x0074b450
XmlElement* SerializerV2::newRootElement(const std::string& type)  // 0x5AF1DA
{
	static const RBX::Name* xmlnsxmime = RBX::Name::declare("xmlns:xmime");
	XmlElement* root = new XmlElement(*tag_roblox);
	root->addAttribute(*xmlnsxmime, "http://www.w3.org/2005/05/xmlmime");
	root->addAttribute(*tag_xmlnsxsi, "http://www.w3.org/2001/XMLSchema-instance");
	root->addAttribute(*tag_xsinoNamespaceSchemaLocation, "http://www.roblox.com/roblox.xsd");
	root->addAttribute(*tag_version, 4);
	if (!type.empty())
		root->addAttribute(*tag_assettype, type);
	root->addChild(new XmlElement(*tag_External, value_IDREF_null));
	root->addChild(new XmlElement(*tag_External, value_IDREF_nil));
	return root;
}

// STUB: PLAYER 0x006fb520
// STUB: STUDIO 0x00749ab0
shared_ptr<RBX::Instance> SerializerV2::loadInstance(const XmlElement* itemElement, RBX::IReferenceBinder& binder, RBX::CreatorRole creatorRole)  // 0x5AEB9E
{
	const RBX::Name* className = NULL;
	if (itemElement->findAttributeValue(*tag_class, className) && className)
	{
		if (shared_ptr<RBX::Instance> inst = RBX::Creatable<RBX::Instance>::createByName(*className, creatorRole))
		{
			inst->read(itemElement, binder, creatorRole);
			return inst;
		}
		RBX::StandardOut::singleton()->print(RBX::MESSAGE_WARNING,
			std::string("Unknown object class \"") + className->c_str() + "\" while reading XML");
	}
	return shared_ptr<RBX::Instance>();
}

// STUB: PLAYER 0x006fc990
// STUB: STUDIO 0x0074af20
void SerializerV2::loadInstances(const XmlElement* root, RBX::Instances& result, RBX::IReferenceBinder& binder, RBX::CreatorRole creatorRole)  // 0x5AEE84
{
	int version = 0;
	const XmlAttribute* versionAttr = root->findAttribute(*tag_version);
	if (root->getTag() == *tag_roblox && versionAttr != NULL
		&& versionAttr->getValue(version) && version >= 4)
	{
		for (const XmlElement* item = root->findFirstChildByTag(*tag_Item);
			item != NULL;
			item = root->findNextChildWithSameTag(item))
		{
			if (shared_ptr<RBX::Instance> inst = loadInstance(item, binder, creatorRole))
				result.push_back(inst);
		}
		binder.resolveRefs();
	}
	else
	{
		// 2013 emits via G3D::Log::println; our vendored (2016-fork) G3D has no Log
		// API (TODO: G3D version archaeology). StandardOut stand-in.
		RBX::StandardOut::singleton()->print(RBX::MESSAGE_WARNING,
			"Version 1.0 - 3.0 model files not supported - all future version will be supported");
	}
}

void SerializerV2::loadInstancesFromText(const XmlElement* root, RBX::Instances& result)  // 0x5AECFC
{
	ArchiveBinder binder;
	loadInstances(root, result, binder, RBX::EngineCreator);
	binder.resolveRefs();
}

void SerializerV2::loadInstancesFromMemory(const XmlElement* root, RBX::Instances& result, RBX::CreatorRole creatorRole)  // 0x5AF018
{
	MemoryBinder binder;
	loadInstances(root, result, binder, creatorRole);
}

void SerializerV2::load(XmlElement* root, RBX::DataModel* dataModel)  // 0x5ADDDA
{
	ArchiveBinder binder;
	dataModel->readChildren(root, binder, RBX::EngineCreator);
	binder.resolveRefs();
}

// STUB: PLAYER 0x006fcc00
// STUB: STUDIO 0x0074b190
void SerializerV2::loadXML(std::istream& stream, RBX::DataModel* dataModel)  // 0x5AE83E
{
	TextXmlParser parser(stream.rdbuf());
	std::auto_ptr<XmlElement> root(parser.parse());
	if (root->getTag() != *tag_roblox)
	{
		schemaVersionLoading = 1;
		throw std::runtime_error("SerializerV2::loadXML ill-formed XML. No Roblox tag");
	}
	const XmlAttribute* versionAttr = root->findAttribute(*tag_version);
	if (versionAttr != NULL)
	{
		if (!versionAttr->getValue(schemaVersionLoading))
			throw std::runtime_error("SerializerV2::loadXML no version number");
		if (schemaVersionLoading <= 3)
			throw std::runtime_error("SerializerV2::loadXML schemaVersionLoading<4");
		load(root.get(), dataModel);
	}
	// TODO(decompile): DataModel vtable+140 call with 0 after load (0x5AE83E tail).
}

// STUB: PLAYER 0x006fd070
// STUB: STUDIO 0x0074b600
void SerializerV2::load(std::istream& stream, RBX::DataModel* dataModel)  // 0x5ADF6C
{
	// Compound/binary/XML dispatcher. Evidence: 0x5ADF6C decompile (see header
	// comment block). Error literals below are binary-verbatim (including the
	// missing space in "canfileFormatVersion!=1").
	char header[8];
	stream.read(header, sizeof(header));
	if (stream.fail())
		throw std::runtime_error("SerializerV2::load can't read header");
	unsigned int magic = 0;
	memcpy(&magic, header, sizeof(magic));
	stream.clear();
	if (magic == 1599619666u)  // old "RBX_" compound format (bytes 52 42 58 5F)
	{
		stream.seekg(4, std::ios::beg);
		unsigned int fileFormatVersion = 0;
		stream.read(reinterpret_cast<char*>(&fileFormatVersion), sizeof(fileFormatVersion));
		if (stream.fail())
			throw std::runtime_error("SerializerV2::load can't read old fileFormatVersion");
		if (fileFormatVersion != 1)
			throw std::runtime_error("SerializerV2::load canfileFormatVersion!=1");
		unsigned int contentPos = 0;
		stream.read(reinterpret_cast<char*>(&contentPos), sizeof(contentPos));
		if (stream.fail())
			throw std::runtime_error("SerializerV2::load can't read contentPos");
		if (contentPos != 0)
		{
			std::streampos savedPos = stream.tellg();
			stream.clear();
			stream.seekg(contentPos, std::ios::beg);
			while (stream.peek() != 0)
			{
				char name[32];
				stream.read(name, sizeof(name));
				if (stream.fail())
					throw std::runtime_error("SerializerV2::load error reading compound file (1)");
				unsigned int size = 0;
				stream.read(reinterpret_cast<char*>(&size), sizeof(size));
				if (stream.fail())
					throw std::runtime_error("SerializerV2::load error reading compound file (2)");
				char flag = 0;
				stream.read(&flag, sizeof(flag));
				if (stream.fail())
					throw std::runtime_error("SerializerV2::load error reading compound file (3)");
				if (flag != 0)
					throw std::runtime_error("SerializerV2::load error reading compound file (4)");
				ContentProvider* provider = NULL;
				if (dataModel != NULL)
					provider = dataModel->find<ContentProvider>();
				if (provider != NULL)
					provider->readContent(name, stream, size);
				else
					stream.ignore(size);
			}
			stream.clear();
			stream.seekg(savedPos);
		}
		char sentinel = 0;
		stream.read(&sentinel, sizeof(sentinel));
		if (stream.fail())
			throw std::runtime_error("SerializerV2::load error reading compound file (5)");
		if (sentinel != 0)
			throw std::runtime_error("SerializerV2::load error reading compound file (6)");
		loadXML(stream, dataModel);
		return;
	}
	stream.seekg(0, std::ios::beg);
	if (memcmp(header, "<roblox!", 8) != 0)
	{
		loadXML(stream, dataModel);
		return;
	}
	RBX::SerializerBinary::deserialize(stream, dataModel);
}
