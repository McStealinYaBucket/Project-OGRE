/* App/v8xml/SerializerV2.cpp -- October 2013, written from the PLAYER bytes.
   The binders are classes local to this file, as in 2013 (their inline bodies carry this file's
   assert lines). Layouts from their out-of-line ctors/dtors:
   - MemoryBinder (ctor 0x006fb6e0, dtor 0x006fc8d0): map<InstanceHandle, InstanceHandle> at +0x04,
     vector<IDREFItem> at +0x10; size 0x20. It defers every IDREF and remaps it in resolveRefs.
   - ArchiveBinder : MergeBinder (ctor 0x006faeb0, dtor 0x006fb460): MergeBinder's vector at +0x04,
     map<std::string, InstanceHandle> at +0x14, list<IDREFBinding> at +0x20; size 0x2c.
   Both vftables (0x00b46db4, 0x00b46e64): announceID, announceIDREF, resolveRefs, dtor, processID, processIDREF. */

#include "stdafx.h"

#include "V8Xml/SerializerV2.h"
#include "V8Xml/TextXmlParser.h"
#include "V8Xml/SerializerBinary.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/ContentProvider.h"
#include "rbx/Debug.h"
#include "util/standardout.h"

#include <algorithm>
#include <functional>
#include <list>

using namespace RBX;

// The assert text in resolveIDREF is "value_IDREF_nil!=s" (2013 compared the tag Name with the
// string directly); this keeps that spelling with this tree's pointer-typed tags.
static inline bool operator!=(const Name* name, const std::string& s) { return name->str != s; }
// IReferenceBinder for in-memory copies (copy/paste, undo): handles are remapped, not looked up by name.
class MemoryBinder : public IReferenceBinder
{
	std::map<InstanceHandle, InstanceHandle> idMap;	// 0x04: original handle -> new instance
	struct IDREFItem {
		const IIDREF* idref;
		Reflection::DescribedBase* propertyOwner;
		InstanceHandle value;
	};
	std::vector<IDREFItem> deferredIDREFItems;		// 0x10

public:
	virtual void announceID(const XmlNameValuePair* valueID, Reflection::DescribedBase* source)
	{
		processID(valueID, source);
	}
	// FUNCTION: PLAYER 0x006fb750
	// FUNCTION: STUDIO 0x00749ce0
	virtual void announceIDREF(const XmlNameValuePair* valueIDREF, Reflection::DescribedBase* propertyOwner, const IIDREF* idref)
	{
		bool processedIDREF = processIDREF(valueIDREF, propertyOwner, idref);
		RBXASSERT(processedIDREF);
	}
	// STUB: PLAYER 0x006fc420
	// STUB: STUDIO 0x0074a9b0
	virtual bool resolveRefs()
	{
		for (std::vector<IDREFItem>::iterator iter = deferredIDREFItems.begin(); iter != deferredIDREFItems.end(); ++iter)
		{
			std::map<InstanceHandle, InstanceHandle>::iterator mapped = idMap.find(iter->value);
			if (mapped != idMap.end())
				assign(iter->idref, iter->propertyOwner, mapped->second);
			else
				assign(iter->idref, iter->propertyOwner, iter->value);
		}
		deferredIDREFItems.clear();
		return true;
	}
protected:
	// FUNCTION: PLAYER 0x006fc500
	// FUNCTION: STUDIO 0x0074aa90
	virtual bool processID(const XmlNameValuePair* valueID, Reflection::DescribedBase* source)
	{
		InstanceHandle h;
		if (valueID->getValue(h))
		{
			idMap[h] = InstanceHandle(shared_from(source));
			return true;
		}
		else if (valueID->isValueEqual(value_IDREF_nil))
			return true;
		return false;
	}
	// FUNCTION: PLAYER 0x006fc720
	// FUNCTION: STUDIO 0x0074acb0
	virtual bool processIDREF(const XmlNameValuePair* valueIDREF, Reflection::DescribedBase* propertyOwner, const IIDREF* idref)
	{
		InstanceHandle value;
		if (valueIDREF->getValue(value))
		{
			IDREFItem item = {idref, propertyOwner, value};
			deferredIDREFItems.push_back(item);
			return true;
		}
		else if (valueIDREF->isValueEqual(value_IDREF_nil))
			return true;
		else
			return false;
	}
};
// IReferenceBinder for archive loads: string referents; unresolved IDREFs are bound by name at the end.
class ArchiveBinder : public MergeBinder
{
	typedef MergeBinder Super;
	std::map<std::string, InstanceHandle> idMap;		// 0x14
	struct IDREFBinding { const XmlNameValuePair* valueIDREF; Reflection::DescribedBase* propertyOwner; const IIDREF* idref; };

	std::list<IDREFBinding> idrefBindings;			// 0x20
public:
	// STUB: PLAYER 0x006fb300
	// STUB: STUDIO 0x00749890
	virtual bool processID(const XmlNameValuePair* valueID, Reflection::DescribedBase* source)
	{
		if (Super::processID(valueID, source))
			return true;
		std::string s;
		bool foundString = valueID->getValue(s);
		RBXASSERT(foundString);
		idMap[s].linkTo(shared_from(source));
		return true;
	}
	// FUNCTION: PLAYER 0x006fb3f0
	// FUNCTION: STUDIO 0x00749980
	virtual bool processIDREF(const XmlNameValuePair* valueIDREF, Reflection::DescribedBase* propertyOwner, const IIDREF* idref)
	{
		if (!Super::processIDREF(valueIDREF, propertyOwner, idref))
		{
			IDREFBinding binding = {valueIDREF, propertyOwner, idref};
			idrefBindings.push_back(binding);
		}
		return true;
	}
	// FUNCTION: PLAYER 0x006fa6e0
	// FUNCTION: STUDIO 0x00748df0
	bool resolveIDREF(IDREFBinding binding)
	{
		std::string s;
		bool foundString = binding.valueIDREF->getValue(s);
		RBXASSERT(foundString);

		// nil, null and "" were all handled by processIDREF
		RBXASSERT(value_IDREF_nil!=s);
		RBXASSERT(value_IDREF_null!=s);
		RBXASSERT(s!="");

		std::map<std::string, InstanceHandle>::iterator found = idMap.find(s);
		if (found != idMap.end())
		{
			assign(binding.idref, binding.propertyOwner, found->second);
			return true;
		}
		assign(binding.idref, binding.propertyOwner, InstanceHandle(NULL));
		return false;
	}

	// FUNCTION: PLAYER 0x006fadd0
	// FUNCTION: STUDIO 0x007494e0
	virtual bool resolveRefs()
	{
		int resolved = 0;
		resolved += std::count_if(idrefBindings.begin(), idrefBindings.end(),
			std::bind1st(std::mem_fun1(&ArchiveBinder::resolveIDREF), this));
		return Super::resolveRefs() && resolved == idrefBindings.size();
	}
};

// FUNCTION: PLAYER 0x006fb520
// FUNCTION: STUDIO 0x00749ab0
shared_ptr<Instance> SerializerV2::loadInstance(const XmlElement* itemElement, IReferenceBinder& binder, CreatorRole creatorRole)
{
	const Name* className = NULL;
	if (itemElement->findAttributeValue(*tag_class, className))
	{
		shared_ptr<Instance> instance = Creatable<Instance>::createByName(*className, SerializationCreator);
		if (instance)
		{
			instance->read(itemElement, binder, creatorRole);
			return instance;
		}
		StandardOut::singleton()->printf(MESSAGE_WARNING, "Unknown object class \"%s\" while reading XML",
			className ? className->c_str() : "");
	}
	return shared_ptr<Instance>();
}

// G3D 8.00's Log (PLAYER Log::common 0x009d5c60, println 0x009d5da0). The G3D fork in
// Client/Rendering has no G3D/Log.h yet, so the two members used here are declared locally.
namespace G3D {
	class Log {
	public:
		static Log* common();
		void println(const std::string& s);
	};
}

// FUNCTION: PLAYER 0x006fc990
// STUB: STUDIO 0x0074af20
void SerializerV2::loadInstances(const XmlElement* root, Instances& result, IReferenceBinder& binder, CreatorRole creatorRole)
{
	if (root->getTag() == *tag_roblox)
	{
		const XmlAttribute* version = root->findAttribute(*tag_version);
		if (version && version->getValue(schemaVersionLoading) && schemaVersionLoading >= 4)
		{
			for (const XmlElement* item = root->findFirstChildByTag(*tag_Item); item; item = root->findNextChildWithSameTag(item))
			{
				shared_ptr<Instance> instance = loadInstance(item, binder, creatorRole);
				if (instance)
					result.push_back(instance);
			}
			binder.resolveRefs();
			return;
		}
	}
	G3D::Log::common()->println("Version 1.0 - 3.0 model files not supported - all future version will be supported");
}

// FUNCTION: PLAYER 0x006fcb70
// FUNCTION: STUDIO 0x0074b100
void SerializerV2::load(XmlElement* root, DataModel* dataModel)
{
	ArchiveBinder binder;
	dataModel->readChildren(root, binder, SerializationCreator);
	binder.resolveRefs();
}

// FUNCTION: PLAYER 0x006fcc00
// FUNCTION: STUDIO 0x0074b190
void SerializerV2::loadXML(std::istream& stream, DataModel* dataModel)
{
	TextXmlParser machine(stream.rdbuf());
	std::auto_ptr<XmlElement> root(machine.parse());
	if (root->getTag() == *tag_roblox)
	{
		if (const XmlAttribute* version = root->findAttribute(*tag_version))
		{
			if (!version || !version->getValue(schemaVersionLoading))
				throw std::runtime_error("SerializerV2::loadXML no version number");
			else if (schemaVersionLoading < 4)
				throw std::runtime_error("SerializerV2::loadXML schemaVersionLoading<4");
			else
				load(root.get(), dataModel);
		}
	}
	else
	{
		schemaVersionLoading = 1;
		throw std::runtime_error("SerializerV2::loadXML ill-formed XML. No Roblox tag");
	}
	dataModel->setDirty(false);
}

// FUNCTION: PLAYER 0x006fcde0
// FUNCTION: STUDIO 0x0074b370
void SerializerV2::loadInstancesFromText(const XmlElement* root, Instances& result)
{
	ArchiveBinder binder;
	loadInstances(root, result, binder, SerializationCreator);
}

// FUNCTION: PLAYER 0x006fce50
// FUNCTION: STUDIO 0x0074b3e0
void SerializerV2::loadInstancesFromMemory(const XmlElement* root, Instances& result, CreatorRole creatorRole)
{
	MemoryBinder binder;
	loadInstances(root, result, binder, creatorRole);
}

// FUNCTION: PLAYER 0x006fcec0
// FUNCTION: STUDIO 0x0074b450
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

// FUNCTION: PLAYER 0x006fd070
// FUNCTION: STUDIO 0x0074b600
void SerializerV2::load(std::istream& stream, RBX::DataModel* dataModel)  // 0x5ADF6C
{
	// Compound/binary/XML dispatcher. Error literals are binary-verbatim (including the missing
	// space in "canfileFormatVersion!=1"). Each read is checked with good() (2013 compares the
	// whole rdstate with 0).
	char header[8];
	if (!stream.read(header, sizeof(header)).good())
		throw std::runtime_error("SerializerV2::load can't read header");
	if (*reinterpret_cast<unsigned int*>(header) == 0x5f584252)  // "RBX_": the old compound format
	{
		stream.clear();
		stream.seekg(4, std::ios::beg);
		unsigned int fileFormatVersion;
		if (!stream.read(reinterpret_cast<char*>(&fileFormatVersion), sizeof(fileFormatVersion)).good())
			throw std::runtime_error("SerializerV2::load can't read old fileFormatVersion");
		if (fileFormatVersion != 1)
			throw std::runtime_error("SerializerV2::load canfileFormatVersion!=1");
		unsigned int contentPos = 0;
		if (!stream.read(reinterpret_cast<char*>(&contentPos), sizeof(contentPos)).good())
			throw std::runtime_error("SerializerV2::load can't read contentPos");
		if (contentPos != 0)
		{
			std::streamoff savedPos = stream.tellg();
			stream.clear();
			stream.seekg(contentPos, std::ios::beg);
			while (stream.peek())
			{
				char name[33];
				name[32] = 0;
				if (!stream.read(name, 32).good())
					throw std::runtime_error("SerializerV2::load error reading compound file (1)");
				unsigned int size;
				if (!stream.read(reinterpret_cast<char*>(&size), sizeof(size)).good())
					throw std::runtime_error("SerializerV2::load error reading compound file (2)");
				char flag;
				if (!stream.read(&flag, sizeof(flag)).good())
					throw std::runtime_error("SerializerV2::load error reading compound file (3)");
				if (flag != 0)
					throw std::runtime_error("SerializerV2::load error reading compound file (4)");
				// readContent returns a std::string that is discarded here (0x006fd2d9).
				ServiceProvider::create<ContentProvider>(dataModel)->readContent(name, stream, size);
			}
			stream.clear();
			stream.seekg(savedPos, std::ios::beg);
		}
		char sentinel;
		if (!stream.read(&sentinel, sizeof(sentinel)).good())
			throw std::runtime_error("SerializerV2::load error reading compound file (5)");
		if (sentinel != 0)
			throw std::runtime_error("SerializerV2::load error reading compound file (6)");
		loadXML(stream, dataModel);
	}
	else
	{
		stream.clear();
		stream.seekg(0, std::ios::beg);
		if (memcmp(header, "<roblox!", 8) == 0)
			RBX::SerializerBinary::deserialize(stream, dataModel);
		else
			loadXML(stream, dataModel);
	}
}

// FUNCTION: PLAYER 0x006fd510
// FUNCTION: STUDIO 0x0074baa0
XmlElement* SerializerV2::newRootElement()  // 0x5AF136
{
	return newRootElement("");
}

/* Marker notes (2026-11-23). Every STUB above except load(istream&) is byte-identical to PLAYER
   under tools/probe_compare.py; reccmp scores them below 100% only because a callee pairs with
   a different original copy:
   - MemoryBinder::resolveRefs: InstanceHandle::operator< (0x006fa4c0) folds with
     std::_String_const_iterator::operator< in our link; the IDREFItem vector helpers fold with
     MergeBinder's, whose originals live in the LTCG client shell (0x0044xxxx).
   - MemoryBinder::processID: InstanceHandle::operator= pairs with the shell's shared_ptr copy
     (0x00469520) in the original, a boost::program_options twin in ours.
   - loadInstance / load(XmlElement*) / loadInstances: Instance::read (0x006215e0),
     Instance::readChildren (0x00621590) and vector<shared_ptr<Instance> >::push_back are not
     paired yet (Instance.cpp); loadInstances also calls G3D::Log, which the Rendering fork lacks.
   - loadXML: the TextXmlParser ctor/dtor and ~XmlElement it calls are client-shell copies in the
     original (0x00402c00, 0x00404e10, 0x00402350).
   load(istream&) itself still needs ContentProvider::readContent's 2013 signature (it returns a
   value the caller discards) and the "RBX_" header compare. */

// MemoryBinder ctor 0x006fb6e0 and ArchiveBinder's bodies (100%).
#include "decomp.h"
DECOMP_SIZE_ASSERT(MemoryBinder, 0x20);
DECOMP_SIZE_ASSERT(ArchiveBinder, 0x2c);
