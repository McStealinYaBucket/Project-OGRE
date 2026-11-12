/* v8xml/SerializerV2.h — October 2013 V2 XML/binary serializer surface.
   Evidence: studio_function_list.txt mangles (all in the global namespace —
     NO RBX:: prefix — except the binder helpers):
      - load(XmlElement*, DataModel*) 0x5ADDDA; load(istream&, DataModel*) 0x5ADF6C;
        loadXML(istream&, DataModel*) 0x5AE83E
      - loadInstance(XmlElement const*, IReferenceBinder&, CreatorRole) 0x5AEB9E
        -> shared_ptr<Instance>
      - loadInstancesFromText(XmlElement const*, Instances&) 0x5AECFC
      - loadInstances(XmlElement const*, Instances&, IReferenceBinder&, CreatorRole)
        0x5AEE84
      - loadInstancesFromMemory(XmlElement const*, Instances&, CreatorRole) 0x5AF018
      - newRootElement() 0x5AF136; newRootElement(string const&) 0x5AF1DA
   DIVERGENCE from March 2016 (private loadInstancesXML/loadInstanceXML +
     load(istream)/loadInstances(istream) only): 2013 exposes the element-based
     load/loadInstance/loadInstances family with different names.
   ArchiveBinder/MemoryBinder (global classes): processID/IDREF + resolveRefs EAs
     in BinderParser.cpp.txt (0x5B10C4/0x5B122C/0x5AF506, 0x5B0332/0x5B0448/0x5B0232).
     ArchiveBinder::processID asserts foundString (SerializerV2.cpp:116).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8xml/SerializerV2.h
   (path inferred from assert + 2016 layout; TODO confirm). */

#pragma once

#include <vector>
#include <list>
#include <string>
#include <map>

#include "V8Xml/XmlElement.h"
#include "V8Tree/Instance.h"
#include "V8Xml/Reference.h"
#include "Util/Object.h"

namespace RBX {
	class DataModel;
}

// TODO: Refactor: Call this RBX::DOM or something
class SerializerV2 {
protected:
	int schemaVersionLoading;
public:
	static const int CURRENT_SCHEMA_VERSION = 4;

	// writing:
	static XmlElement* newRootElement();						// 0x5AF136
	static XmlElement* newRootElement(const std::string& type);	// 0x5AF1DA

	// reading:
	void loadInstancesFromText(const XmlElement* root, RBX::Instances& result);	// 0x5AECFC
	void loadInstancesFromMemory(const XmlElement* root, RBX::Instances& result, RBX::CreatorRole creatorRole);	// 0x5AF018

	// Until DataModel becomes an Instance and it can handle "globals" like Workspace, we need to treat
	// it specially during reads:
	void load(XmlElement* root, RBX::DataModel* dataModel);		// 0x5ADDDA
	void load(std::istream& stream, RBX::DataModel* dataModel);	// 0x5ADF6C

	void loadInstances(const XmlElement* root, RBX::Instances& result, RBX::IReferenceBinder& binder, RBX::CreatorRole creatorRole);	// 0x5AEE84
	shared_ptr<RBX::Instance> loadInstance(const XmlElement* itemElement, RBX::IReferenceBinder& binder, RBX::CreatorRole creatorRole);	// 0x5AEB9E

private:
	void loadXML(std::istream& stream, RBX::DataModel* dataModel);	// 0x5AE83E
};

// An implementation of IReferenceBinder for archive (file) loads. Extends the
// MergeBinder IDREF machinery with a string->handle ID map + setXmlId wiring
// (processID asserts foundString, SerializerV2.cpp:116).
class ArchiveBinder : public RBX::MergeBinder
{
private:
	typedef RBX::MergeBinder Super;
	// A map of temporary IDs to Referents
	std::map<std::string, RBX::InstanceHandle> idMap;

public:
	virtual bool processID(const XmlNameValuePair* valueID, RBX::Reflection::DescribedBase* source);
	virtual bool processIDREF(const XmlNameValuePair* valueIDREF, RBX::Reflection::DescribedBase* propertyOwner, const RBX::IIDREF* idref);
	virtual bool resolveRefs();
};

// Binder for in-memory merges (copy/paste, undo): remaps handles instead of strings.
class MemoryBinder : public RBX::IReferenceBinder
{
public:
	struct IDREFItem {
		const RBX::IIDREF* idref;
		RBX::Reflection::DescribedBase* propertyOwner;
		RBX::InstanceHandle value;
	};
	std::vector<IDREFItem> deferredIDREFItems;
	std::map<RBX::InstanceHandle, RBX::InstanceHandle> idMap;

	virtual void announceID(const XmlNameValuePair* valueID, RBX::Reflection::DescribedBase* source);
	virtual void announceIDREF(const XmlNameValuePair* valueIDREF, RBX::Reflection::DescribedBase* propertyOwner, const RBX::IIDREF* idref);
	virtual bool resolveRefs();
	virtual bool processID(const XmlNameValuePair* valueID, RBX::Reflection::DescribedBase* source);
	virtual bool processIDREF(const XmlNameValuePair* valueIDREF, RBX::Reflection::DescribedBase* propertyOwner, const RBX::IIDREF* idref);
};
