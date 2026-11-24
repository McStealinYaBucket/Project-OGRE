/* ClientBase/ReflectionMetadata.h -- October 2013 client shell (PLAYER and STUDIO compile it).
   The "ReflectionMetadata*" Instance classes that read ReflectionMetadata.xml (class documentation:
   browsable/deprecated flags, Explorer order and image). The 2013 file is
   Client\ClientBase\ReflectionMetadata.cpp (assert `bound`, line 118).

   2013 against the 2016 header: no Enums/Enum/EnumItem classes, no Item::minimum/maximum
   ("UIMinimum"/"UIMaximum" are not in the binary), no Class::insertable ("Insertable" absent).
   Layouts from the PLAYER bytes (the shell is /GL, so ctors take `this` on the stack):
     Reflection 0x64   create<Reflection> 0x0040d7b0 (operator new 0x64, classes = 0 at +0x60)
     Classes    0x60   create<Classes> 0x0040d850
     Item       0x80   ctor 0x00402d10 (+0x60 browsable = 1, +0x61/+0x62 = 0, string at +0x64), dtor 0x00402db0
     Class      0xa4   ctor 0x00402e20 (+0x80 = -1, +0x84 = 0, preferredParent("") at +0x88)
     Member     0x80   create<Member> 0x0040dc90
     Properties ... Callbacks derive Members (an Instance, inline ctor), 0x60 each. */

#pragma once

#include <ostream>
#include <string>

#include "V8Tree/Instance.h"
#include "Reflection/Reflection.h"

namespace RBX
{
	namespace Reflection
	{
		namespace Metadata
		{
			// The API dump (inlined into the shell's command-line handler, PLAYER 0x004054b8).
			void writeEverything(std::ostream& stream);

			class Classes;
			class Class;
			class Member;

			extern const char* const sReflection;
			// VTABLE: PLAYER 0x00be2e64
			// SIZE 0x64
			class Reflection : public DescribedCreatable<Reflection, Instance, sReflection>
			{
				Classes* classes;  // 0x60
				static boost::shared_ptr<Reflection> safe_static_do_get_singleton();
				// FUNCTION: PLAYER 0x004478d0
				static void safe_static_init_singleton() { safe_static_do_get_singleton(); }

			public:
				Reflection() : classes(0) {}

				// FUNCTION: PLAYER 0x00447920
				static boost::shared_ptr<Reflection> singleton()
				{
					static boost::once_flag once_init_singleton = BOOST_ONCE_INIT;
					boost::call_once(safe_static_init_singleton, once_init_singleton);
					return safe_static_do_get_singleton();
				}

				void load(std::string filePath);
				// findBestMatch: fall back to the nearest base class that has metadata
				Class* get(const ClassDescriptor& descriptor, bool findBestMatch) const;
				Member* get(const PropertyDescriptor& descriptor) const;
				Member* get(const FunctionDescriptor& descriptor) const;
				Member* get(const YieldFunctionDescriptor& descriptor) const;
				Member* get(const EventDescriptor& descriptor) const;
				Member* get(const CallbackDescriptor& descriptor) const;
			};

			extern const char* const sClasses;
			// VTABLE: PLAYER 0x00bd9444
			// SIZE 0x60
			class Classes : public DescribedCreatable<Classes, Instance, sClasses>
			{
			public:
				Classes() {}
				Class* get(const ClassDescriptor& descriptor, bool findBestMatch);
			};

			extern const char* const sItem;
			// VTABLE: PLAYER 0x00bd96b4
			// SIZE 0x80
			class Item : public DescribedNonCreatable<Item, Instance, sItem>
			{
				friend class Classes;
				bool browsable;   // 0x60
				bool deprecated;  // 0x61
				bool backend;     // 0x62
			public:
				std::string description;  // 0x64

				static bool isDeprecated(const Item* item, const Descriptor& descriptor)
				{
					if (item && item->deprecated)
						return true;
					return descriptor.isDeprecated;
				}
				bool isBrowsable() const { return browsable; }
				static bool isBackend(const Item* item, const Descriptor& descriptor)
				{
					if (item && item->backend)
						return true;
					return false;
				}

				// PLAYER 0x00402d10 (out of line in the shell); the implicit dtor is 0x00402db0.
				Item() : browsable(true), deprecated(false), backend(false) {}

				static BoundProp<bool> prop_browsable;
				static BoundProp<bool> prop_deprecated;
				static BoundProp<bool> prop_backend;
				static BoundProp<std::string> prop_description;
			};

			extern const char* const sClass;
			// VTABLE: PLAYER 0x00bd94dc
			// SIZE 0xa4
			class Class : public DescribedCreatable<Class, Item, sClass>
			{
				int explorerOrder;            // 0x80 (-1 hide, 0 unspecified)
				int explorerImageIndex;       // 0x84
				std::string preferredParent;  // 0x88
			public:
				static BoundProp<int> prop_ExplorerOrder;
				static BoundProp<int> prop_ExplorerImageIndex;
				static BoundProp<std::string> prop_PreferredParent;

				// PLAYER 0x00402e20 (out of line in the shell).
				Class() : explorerOrder(-1), explorerImageIndex(0), preferredParent("") {}
			};

			class Members : public Instance
			{
			public:
				Members() {}
			};

			extern const char* const sProperties;
			// VTABLE: PLAYER 0x00bd9884
			// SIZE 0x60
			class Properties : public DescribedCreatable<Properties, Members, sProperties>
			{
			public:
				Properties() {}
			};

			extern const char* const sFunctions;
			// VTABLE: PLAYER 0x00bd99bc
			// SIZE 0x60
			class Functions : public DescribedCreatable<Functions, Members, sFunctions>
			{
			public:
				Functions() {}
			};

			extern const char* const sYieldFunctions;
			// VTABLE: PLAYER 0x00bd9af4
			// SIZE 0x60
			class YieldFunctions : public DescribedCreatable<YieldFunctions, Members, sYieldFunctions>
			{
			public:
				YieldFunctions() {}
			};

			extern const char* const sEvents;
			// VTABLE: PLAYER 0x00bd9c2c
			// SIZE 0x60
			class Events : public DescribedCreatable<Events, Members, sEvents>
			{
			public:
				Events() {}
			};

			extern const char* const sCallbacks;
			// VTABLE: PLAYER 0x00bd9d64
			// SIZE 0x60
			class Callbacks : public DescribedCreatable<Callbacks, Members, sCallbacks>
			{
			public:
				Callbacks() {}
			};

			extern const char* const sMember;
			// VTABLE: PLAYER 0x00bd9574
			// SIZE 0x80
			class Member : public DescribedCreatable<Member, Item, sMember>
			{
			public:
				Member() {}
			};
		}
	}
}
