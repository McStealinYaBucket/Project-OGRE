/* App/v8xml/WebSerializer.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
     (F:\2013Decompile\exports\decomp\SerializerV2_full.cpp.txt)
      - writeTable 0x5FCDBE: WebTable element + writeEntry per pair + addChild.
      - writeEntry 0x5FCE56: writeValue -> WebEntry + WebKey-tagged string child +
        value child (null value -> NULL entry).
      - writeList 0x5FCF9E: WebList + writeValue per item + addChild.
      - writeValue 0x5FD024: type dispatch on Variant typeinfo — double/float ->
        WebValue + WebType="number"; string -> WebValue + WebType="string"(literal
        TODO); bool -> WebValue + WebType="boolean" + true/false text; null
        shared_ptr<Instance> -> NULL; live one -> toNewXmlRoot + WebValue +
        WebType="instance" + child; map -> writeTable + WebType="table";
        vector -> writeList + WebType="list"(literal TODO); else NULL.
   TODO(decompile): exact type-attr literals ("string" assumed by parallel structure;
     "number"/"boolean"/"instance"/"table" are binary literals); WebKey tag identity
     (address-derived, tag_WebKey assumed); CreatorRole numeric value in toNewXmlRoot.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8xml/WebSerializer.cpp */

#include "stdafx.h"

#include "V8Xml/WebSerializer.h"
#include "V8Xml/XmlElement.h"
#include "V8Tree/Instance.h"

namespace RBX {

	XmlElement* WebSerializer::writeTable(const Reflection::ValueMap& result)
	{
		XmlElement* table = new XmlElement(*tag_WebTable);
		for (Reflection::ValueMap::const_iterator iter = result.begin(); iter != result.end(); ++iter)
		{
			if (XmlElement* entry = writeEntry(iter->first, iter->second))
				table->addChild(entry);
		}
		return table;
	}

	XmlElement* WebSerializer::writeEntry(const std::string& key, const Reflection::Variant& value)
	{
		XmlElement* valueElement = writeValue(value);
		if (valueElement == NULL)
			return NULL;
		XmlElement* entry = new XmlElement(*tag_WebEntry);
		XmlElement* keyElement = new XmlElement(*tag_WebKey);
		keyElement->setValue(key);
		entry->addChild(keyElement);
		entry->addChild(valueElement);
		return entry;
	}

	XmlElement* WebSerializer::writeList(const Reflection::ValueArray& result)
	{
		XmlElement* list = new XmlElement(*tag_WebList);
		for (Reflection::ValueArray::const_iterator iter = result.begin(); iter != result.end(); ++iter)
		{
			if (XmlElement* valueElement = writeValue(*iter))
				list->addChild(valueElement);
		}
		return list;
	}

	// STUB: PLAYER 0x0081f410
	// STUB: STUDIO 0x00912ab0
	XmlElement* WebSerializer::writeValue(const Reflection::Variant& value)
	{
		if (value.isType<double>() || value.isType<float>())
		{
			XmlElement* element = new XmlElement(*tag_WebValue);
			element->addAttribute(*tag_WebType, "number");
			Reflection::Variant v = value;
			element->setValue(v.convert<std::string>());
			return element;
		}
		else if (value.isType<std::string>())
		{
			XmlElement* element = new XmlElement(*tag_WebValue);
			element->addAttribute(*tag_WebType, "string");
			Reflection::Variant v = value;
			element->setValue(v.convert<std::string>());
			return element;
		}
		else if (value.isType<bool>())
		{
			XmlElement* element = new XmlElement(*tag_WebValue);
			element->addAttribute(*tag_WebType, "boolean");
			Reflection::Variant v = value;
			element->setValue(v.convert<std::string>());
			return element;
		}
		else if (value.isType<shared_ptr<Instance> >())
		{
			Reflection::Variant v = value;
			shared_ptr<Instance> instance = v.convert<shared_ptr<Instance> >();
			if (instance == NULL)
				return NULL;
			if (XmlElement* root = Instance::toNewXmlRoot(instance.get(), EngineCreator))
			{
				XmlElement* element = new XmlElement(*tag_WebValue);
				element->addAttribute(*tag_WebType, "instance");
				element->addChild(root);
				return element;
			}
			return NULL;
		}
		else if (value.isType<shared_ptr<const Reflection::ValueMap> >())
		{
			Reflection::Variant v = value;
			shared_ptr<const Reflection::ValueMap> table = v.convert<shared_ptr<const Reflection::ValueMap> >();
			if (XmlElement* tableElement = writeTable(*table))
			{
				XmlElement* element = new XmlElement(*tag_WebValue);
				element->addAttribute(*tag_WebType, "table");
				element->addChild(tableElement);
				return element;
			}
			return NULL;
		}
		else if (value.isType<shared_ptr<const Reflection::ValueArray> >())
		{
			Reflection::Variant v = value;
			shared_ptr<const Reflection::ValueArray> list = v.convert<shared_ptr<const Reflection::ValueArray> >();
			if (XmlElement* listElement = writeList(*list))
			{
				XmlElement* element = new XmlElement(*tag_WebValue);
				element->addAttribute(*tag_WebType, "list");
				element->addChild(listElement);
				return element;
			}
			return NULL;
		}
		return NULL;
	}

} // namespace RBX
