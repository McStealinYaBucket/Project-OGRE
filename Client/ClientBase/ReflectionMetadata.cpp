/* ClientBase/ReflectionMetadata.cpp -- October 2013 client shell (PLAYER and STUDIO compile it).
   2013 path from the assert in Reflection::load (`bound`, line 118). The shell is /GL: the
   linker generates these bodies through LTCG (custom register conventions, no .obj order).

   2013 dynamic initialisers (PLAYER 0x00a5b6e0-0x00a5b8a0), in this order: the FLog channel
   (default 1, .data 0x00dc6c1f), Item's Browsable/Deprecated/IsBackend (+0x60/+0x61/+0x62) and
   summary, Class's ExplorerOrder/ExplorerImageIndex (+0x80/+0x84) and PreferredParent, all in
   category "Reflection". The API writer (Writer, writeEverything) follows the lookups. */

#include "ReflectionMetadata.h"

#include <windows.h>
#include <atlpath.h>
#include <fstream>
#include <memory>

#include "rbx/Debug.h"
#include "Util/Limits.h"
#include "v8xml/TextXmlParser.h"
#include "v8xml/Reference.h"
#include "FastLog.h"

namespace RBX
{
	// App's (PLAYER 0x006131f0, native): writes name.str. Not declared by the tree's Util/Name.h.
	std::ostream& operator<<(std::ostream& stream, const Name& name);
}

LOGGROUP(ReflectionMetadata)
LOGVARIABLE(ReflectionMetadata, 1);

using namespace RBX;
using namespace RBX::Reflection;

const char* const Metadata::sReflection = "ReflectionMetadata";
const char* const Metadata::sClasses = "ReflectionMetadataClasses";
const char* const Metadata::sItem = "ReflectionMetadataItem";
const char* const Metadata::sClass = "ReflectionMetadataClass";
const char* const Metadata::sProperties = "ReflectionMetadataProperties";
const char* const Metadata::sFunctions = "ReflectionMetadataFunctions";
const char* const Metadata::sYieldFunctions = "ReflectionMetadataYieldFunctions";
const char* const Metadata::sEvents = "ReflectionMetadataEvents";
const char* const Metadata::sCallbacks = "ReflectionMetadataCallbacks";
const char* const Metadata::sMember = "ReflectionMetadataMember";

BoundProp<bool> Metadata::Item::prop_browsable("Browsable", "Reflection", &Metadata::Item::browsable);
BoundProp<bool> Metadata::Item::prop_deprecated("Deprecated", "Reflection", &Metadata::Item::deprecated);
BoundProp<bool> Metadata::Item::prop_backend("IsBackend", "Reflection", &Metadata::Item::backend);
BoundProp<std::string> Metadata::Item::prop_description("summary", "Reflection", &Metadata::Item::description);

BoundProp<int> Metadata::Class::prop_ExplorerOrder("ExplorerOrder", "Reflection", &Metadata::Class::explorerOrder);
BoundProp<int> Metadata::Class::prop_ExplorerImageIndex("ExplorerImageIndex", "Reflection", &Metadata::Class::explorerImageIndex);
BoundProp<std::string> Metadata::Class::prop_PreferredParent("PreferredParent", "Reflection", &Metadata::Class::preferredParent);

RBX_REGISTER_CLASS(Metadata::Reflection);
RBX_REGISTER_CLASS(Metadata::Class);
RBX_REGISTER_CLASS(Metadata::Item);
RBX_REGISTER_CLASS(Metadata::Classes);
RBX_REGISTER_CLASS(Metadata::Functions);
RBX_REGISTER_CLASS(Metadata::YieldFunctions);
RBX_REGISTER_CLASS(Metadata::Member);
RBX_REGISTER_CLASS(Metadata::Properties);
RBX_REGISTER_CLASS(Metadata::Events);
RBX_REGISTER_CLASS(Metadata::Callbacks);
// PLAYER loads from the exe's folder (STUDIO from RobloxSettings' resources folder, not written).
// STUB: PLAYER 0x00447970
// STUB: STUDIO 0x004338a0
boost::shared_ptr<Metadata::Reflection> Metadata::Reflection::safe_static_do_get_singleton()
{
	static boost::shared_ptr<Reflection> sing;
	if (!sing)
	{
		boost::shared_ptr<Limits::Counter> noCounter;
		Limits::Counter::Activator activator(noCounter);
		sing = Creatable<Instance>::create<Reflection>();

		char buf[MAX_PATH] = {0};
		GetModuleFileNameA(NULL, buf, MAX_PATH);
		ATL::CPath path(buf);
		path.RemoveFileSpec();
		sing->load((const char*)(path + "\\ReflectionMetadata.xml"));
	}
	return sing;
}

/* Reflection::load, PLAYER 0x00447c10 (`this` in edi, the path by value: ret 0x1c).
   The 2013 body, in order:
     - an empty path returns at once;
     - FastLogS(FLog::ReflectionMetadata) with the path;
     - an ifstream opened in|binary (_SH_DENYNO);
     - a TextXmlParser on its filebuf (the shell copy of the inline ctor, 0x00402c00), whose
       parse() (0x00628f70) result is moved into an auto_ptr<XmlElement>;
     - a MergeBinder on the stack (vftable 0x00b31154);
     - Instance::readChildren(root, binder, EngineCreator) (0x00621590);
     - binder.resolveRefs() (0x004474c0) and the assert on its result, line 118;
     - classes = findFirstChildOfType<Classes>() (0x0044c3d0, `this` in eax).
   2016 also saved the tree and looked up Enums; neither is in the 2013 strings. The line of the assert is the 2013 one (layout, never #line). */

// FUNCTION: PLAYER 0x00447c10
// FUNCTION: STUDIO 0x00433aa0
void Metadata::Reflection::load(std::string filePath)
{
	if (filePath.empty())
		return;

	FASTLOGS(FLog::ReflectionMetadata, "Reflection::load %s", filePath.c_str());

	std::ifstream stream(filePath.c_str(), std::ios_base::in | std::ios_base::binary);

	TextXmlParser machine(stream.rdbuf());
	std::auto_ptr<XmlElement> root(machine.parse());

	MergeBinder binder;

	readChildren(root.get(), binder, EngineCreator);

	bool bound = binder.resolveRefs();
	RBXASSERT(bound);

	classes = findFirstChildOfType<Classes>();
}

Metadata::Class* Metadata::Reflection::get(const ClassDescriptor& descriptor, bool findBestMatch) const
{
	return classes ? classes->get(descriptor, findBestMatch) : 0;
}

Metadata::Class* Metadata::Classes::get(const ClassDescriptor& descriptor, bool findBestMatch)
{
	Metadata::Class* c = boost::polymorphic_downcast<Metadata::Class*>(findFirstChildByName(descriptor.name.str));
	if (c)
		return c;
	if (!findBestMatch)
		return NULL;
	if (!descriptor.getBase())
		return NULL;
	return get(*descriptor.getBase(), true);
}

// The five member lookups: Class by the owner's name (no base-class fallback), then the
// Members folder of the descriptor's kind, then the Member by name. The 2013 lookups pass the
// Name's string by reference (no copy, no EH frame), so they read name->str; App's Name has
// toString() return a copy (2016), where 2013's most likely returned a const reference. get(Property) and get(Event)
// stay out of line in the shell (`this` in eax, the descriptor in esi); the other three are
// inlined into their writers.

// FUNCTION: PLAYER 0x00447ec0
Metadata::Member* Metadata::Reflection::get(const PropertyDescriptor& descriptor) const
{
	Metadata::Class* c = get(descriptor.owner, false);
	const Metadata::Properties* members = c ? c->findFirstChildOfType<Metadata::Properties>() : 0;
	return members ? boost::polymorphic_downcast<Metadata::Member*>(members->findFirstChildByNameDangerous(descriptor.name.str)) : 0;
}

Metadata::Member* Metadata::Reflection::get(const FunctionDescriptor& descriptor) const
{
	Metadata::Class* c = get(descriptor.owner, false);
	const Metadata::Functions* members = c ? c->findFirstChildOfType<Metadata::Functions>() : 0;
	return members ? boost::polymorphic_downcast<Metadata::Member*>(members->findFirstChildByNameDangerous(descriptor.name.str)) : 0;
}

Metadata::Member* Metadata::Reflection::get(const YieldFunctionDescriptor& descriptor) const
{
	Metadata::Class* c = get(descriptor.owner, false);
	const Metadata::YieldFunctions* members = c ? c->findFirstChildOfType<Metadata::YieldFunctions>() : 0;
	return members ? boost::polymorphic_downcast<Metadata::Member*>(members->findFirstChildByNameDangerous(descriptor.name.str)) : 0;
}

// FUNCTION: PLAYER 0x00447f00
Metadata::Member* Metadata::Reflection::get(const EventDescriptor& descriptor) const
{
	Metadata::Class* c = get(descriptor.owner, false);
	const Metadata::Events* members = c ? c->findFirstChildOfType<Metadata::Events>() : 0;
	return members ? boost::polymorphic_downcast<Metadata::Member*>(members->findFirstChildByNameDangerous(descriptor.name.str)) : 0;
}

Metadata::Member* Metadata::Reflection::get(const CallbackDescriptor& descriptor) const
{
	Metadata::Class* c = get(descriptor.owner, false);
	const Metadata::Callbacks* members = c ? c->findFirstChildOfType<Metadata::Callbacks>() : 0;
	return members ? boost::polymorphic_downcast<Metadata::Member*>(members->findFirstChildByNameDangerous(descriptor.name.str)) : 0;
}

// The API dump. 2013 has no conflict report, no " [noyield]", no enum metadata and no Plugin
// security level (its jump table, PLAYER 0x00448034, has six cases).
class Writer
{
	std::ostream& stream;
public:
	Writer(std::ostream& stream) : stream(stream) {}

	// FUNCTION: PLAYER 0x00447f40
	void writeMetadata(const Metadata::Item* metadata, const Descriptor& descriptor)
	{
		if (metadata && !metadata->isBrowsable())
			stream << " [notbrowsable]";
		if (Metadata::Item::isDeprecated(metadata, descriptor))
			stream << " [deprecated]";
		if (Metadata::Item::isBackend(metadata, descriptor))
			stream << " [backend]";
	}

	// The 2013 Security::Permissions values: None 0, RobloxPlace 1, LocalUser 2, WritePlayer 3,
	// RobloxScript 4, Roblox 5. App's security/SecurityContext.h has the 2016 list (Plugin = 1),
	// so the cases are spelled as 2013 numbers until that header is corrected.
	// FUNCTION: PLAYER 0x00447fa0
	void writeMemberInfo(const MemberDescriptor* d)
	{
		switch ((int)d->security)
		{
		default:
			stream << " [security" << d->security << "]";
			break;
		case 0:  // None
			break;
		case 1:  // RobloxPlace
			stream << " [RobloxPlaceSecurity]";
			break;
		case 4:  // RobloxScript
			stream << " [RobloxScriptSecurity]";
			break;
		case 2:  // LocalUser
			stream << " [LocalUserSecurity]";
			break;
		case 3:  // WritePlayer
			stream << " [WritePlayerSecurity]";
			break;
		case 5:  // Roblox
			stream << " [RobloxSecurity]";
			break;
		}
	}

	// STUB: PLAYER 0x00448050
	void writeProperty(const PropertyDescriptor* d, const ClassDescriptor* c)
	{
		if (d->owner != *c)
			return;
		if (!d->isScriptable())
			return;
		stream << "\tProperty " << d->type.name.c_str() << ' ' << d->owner.name.c_str() << '.' << d->name.c_str();
		writeMetadata(Metadata::Reflection::singleton()->get(*d), *d);
		if (!d->isPublic())
			stream << " [hidden]";
		if (d->isReadOnly())
			stream << " [readonly]";
		if (d->isWriteOnly())
			stream << " [writeonly]";
		writeMemberInfo(d);
		stream << '\n';
	}

	// FUNCTION: PLAYER 0x00448210
	void writeArg(const SignatureDescriptor::Item& arg, bool* first)
	{
		if (*first)
			*first = false;
		else
			stream << ", ";

		stream << arg.type->name.c_str() << ' ' << arg.name->c_str();
		if (arg.hasDefaultValue())
		{
			stream << " = ";
			stream << arg.defaultValue.get<std::string>();
		}
	}

	// FUNCTION: PLAYER 0x00448320
	void writeArgs(const SignatureDescriptor& d)
	{
		bool first = true;
		std::for_each(d.arguments.begin(), d.arguments.end(), boost::bind(&Writer::writeArg, this, _1, &first));
	}

	// FUNCTION: PLAYER 0x00448370
	void writeFunction(const FunctionDescriptor* d, const ClassDescriptor* c)
	{
		if (d->owner != *c)
			return;
		stream << "\tFunction " << d->getSignature().resultType->name.c_str() << ' ' << d->owner.name.c_str() << ':' << d->name.c_str() << '(';
		writeArgs(d->getSignature());
		stream << ')';
		writeMetadata(Metadata::Reflection::singleton()->get(*d), *d);
		writeMemberInfo(d);
		stream << '\n';
	}

	// FUNCTION: PLAYER 0x00448560
	void writeYieldFunction(const YieldFunctionDescriptor* d, const ClassDescriptor* c)
	{
		if (d->owner != *c)
			return;
		stream << "\tYieldFunction " << d->getSignature().resultType->name.c_str() << ' ' << d->owner.name.c_str() << ':' << d->name.c_str() << '(';
		writeArgs(d->getSignature());
		stream << ')';
		writeMetadata(Metadata::Reflection::singleton()->get(*d), *d);
		writeMemberInfo(d);
		stream << '\n';
	}

	// STUB: PLAYER 0x00448750
	void writeEvent(const EventDescriptor* d, const ClassDescriptor* c)
	{
		if (d->owner != *c)
			return;
		if (!d->isScriptable())
			return;
		stream << "\tEvent " << d->owner.name.c_str() << '.' << d->name.c_str() << '(';
		writeArgs(d->getSignature());
		stream << ')';
		writeMetadata(Metadata::Reflection::singleton()->get(*d), *d);
		writeMemberInfo(d);
		stream << '\n';
	}

	// FUNCTION: PLAYER 0x004488c0
	void writeCallback(const CallbackDescriptor* d, const ClassDescriptor* c)
	{
		if (d->owner != *c)
			return;
		stream << "\tCallback " << d->getSignature().resultType->name.c_str() << ' ' << d->owner.name.c_str() << '.' << d->name.c_str() << '(';
		writeArgs(d->getSignature());
		stream << ')';
		writeMetadata(Metadata::Reflection::singleton()->get(*d), *d);
		writeMemberInfo(d);
		stream << '\n';
	}

	// The class header is written in place (2016 split it out as writeClassHeader for its
	// conflict report; 2013 inlines it here, PLAYER 0x00448b0e-0x00448c05).
	// FUNCTION: PLAYER 0x00448ab0
	void writeClass(const ClassDescriptor* d)
	{
		if (*d != ClassDescriptor::rootDescriptor())
		{
			stream << "Class " << d->name.c_str();
			if (d->getBase() && *d->getBase() != ClassDescriptor::rootDescriptor())
				stream << " : " << d->getBase()->name.c_str();

			writeMetadata(Metadata::Reflection::singleton()->get(*d, false), *d);
			if (!d->isScriptCreatable())
				stream << " [notCreatable]";
			stream << '\n';

			std::for_each(d->begin<PropertyDescriptor>(), d->end<PropertyDescriptor>(), boost::bind(&Writer::writeProperty, this, _1, d));
			std::for_each(d->begin<FunctionDescriptor>(), d->end<FunctionDescriptor>(), boost::bind(&Writer::writeFunction, this, _1, d));
			std::for_each(d->begin<YieldFunctionDescriptor>(), d->end<YieldFunctionDescriptor>(), boost::bind(&Writer::writeYieldFunction, this, _1, d));
			std::for_each(d->begin<EventDescriptor>(), d->end<EventDescriptor>(), boost::bind(&Writer::writeEvent, this, _1, d));
			std::for_each(d->begin<CallbackDescriptor>(), d->end<CallbackDescriptor>(), boost::bind(&Writer::writeCallback, this, _1, d));
		}

		std::for_each(d->derivedClasses_begin(), d->derivedClasses_end(), boost::bind(&Writer::writeClass, this, _1));
	}

	// FUNCTION: PLAYER 0x00448d10
	void writeEnum(const EnumDescriptor* d)
	{
		stream << "Enum " << d->name.c_str() << '\n';
		std::for_each(d->begin(), d->end(), boost::bind(&Writer::writeEnumItem, this, _1));
	}

	// FUNCTION: PLAYER 0x00448d80
	void writeEnumItem(const EnumDescriptor::Item* d)
	{
		stream << "\tEnumItem " << d->owner.name << '.' << d->name << " : " << d->value << '\n';
	}

	// STUB: PLAYER 0x00448df0
	void writeAPI()
	{
		writeClass(&ClassDescriptor::rootDescriptor());
		std::for_each(EnumDescriptor::enumsBegin(), EnumDescriptor::enumsEnd(), boost::bind(&Writer::writeEnum, this, _1));
	}
};

void Metadata::writeEverything(std::ostream& stream)
{
	Writer w(stream);
	w.writeAPI();
}
