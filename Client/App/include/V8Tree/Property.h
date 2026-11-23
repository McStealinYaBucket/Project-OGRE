/* V8Tree/Property.h -- October 2013 reconstruction.
   PropertyChanged, the argument of the 2013 onChildChanged/propertyChanged
   signals (iOS 10-29: RBX::Instance::onChildChanged(RBX::Instance*,
   RBX::PropertyChanged const&) and its overrides in DataModel, Lighting,
   ModelInstance, Players, DebuggerManager), plus the property category names.
   Each category string exists exactly once, NUL-delimited, in PLAYER
   ("Data", "Behavior", "State", "Appearance", "Team", "Image", "Video",
   "Control"). */
#pragma once

#include "Reflection/Property.h"
#include "V8Xml/Reference.h"
#include "V8Xml/XmlElement.h"
#include "Util/Object.h"

#include "G3D/Vector3.h"
#include "G3D/Color3.h"
#include <string>
#include <iostream>
#include <vector>

namespace RBX {

// A copy of the property that changed. Only Instance creates one.
class PropertyChanged
{
	Reflection::Property prop;

	friend class Instance;
	PropertyChanged(const Reflection::Property& p) : prop(p) {}

public:
	PropertyChanged(const PropertyChanged& rhs) : prop(rhs.prop) {}

	const Reflection::Property& getProperty() const { return prop; }
	const Reflection::PropertyDescriptor& getDescriptor() const { return prop.getDescriptor(); }
	const Name& getName() { return prop.getName(); }
};

// Category names shown in the property grid.
#define category_Data       "Data"
#define category_Behavior   "Behavior"
#define category_State      "State"
#define category_Appearance "Appearance"
#define category_Team       "Team"
#define category_Image      "Image"
#define category_Video      "Video"
#define category_Control    "Control"

} // namespace RBX
