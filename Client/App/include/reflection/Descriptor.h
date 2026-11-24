/* reflection/Descriptor.h -- October 2013. Layout { vftable, bool isDeprecated(+4), const Descriptor* preferred(+8),
   const Name& name(+12) } (ctor 0x15F1B0 Mac, dtor 0x44D670). 2016 wraps the first two in Attributes and adds
   scoped_ptr<bool> isReplicable/isOutdated, which 2013 lacks. 2013 asserts !this->name.empty() at line 58,
   and reflection_property.cpp:92 !type.name.empty(): name is a reference to an interned Name. */

#pragma once

#include "Util/Name.h"
#include "boost/utility.hpp"
#include "rbx/Debug.h"

namespace RBX
{
	namespace Reflection
	{
		class Descriptor : public boost::noncopyable
		{
			static void checkLockedDown()
			{
				// Fails when a class is not registered in FactoryRegistrator; severe enough to crash in production.
				if (lockedDown)
					RBXCRASH();
			}

		public:
			struct Attributes
			{
				bool isDeprecated;
				const Descriptor* preferred;	// used if isDeprecated
				Attributes()
					:isDeprecated(false)
					,preferred(NULL)
				{}
				static Attributes deprecated(const Descriptor& preferred)
				{
					Attributes result; result.isDeprecated = true; result.preferred = &preferred; return result;
				}
				static Attributes deprecated()
				{
					Attributes result; result.isDeprecated = true; return result;
				}
			};

			static bool lockedDown;	// after the first instance of a described class is created we cannot modify the reflection database

			const bool isDeprecated;				// +4 (flattened Attributes member in the binary)
			const Descriptor* const preferred;		// +8
			const RBX::Name& name;					// +12

			// FUNCTION: PLAYER 0x00455710
			// FUNCTION: STUDIO 0x004113e0
			Descriptor(const char* name, Attributes attributes)
				:isDeprecated(attributes.isDeprecated)
				,preferred(attributes.preferred)
				,name(*RBX::Name::declare(name))
			{
				checkLockedDown();
				RBXASSERT(!this->name.empty());
			}

			// STUB: PLAYER 0x00455810
			// STUB: STUDIO 0x00407c00
			virtual ~Descriptor() {}
		};
	}
}
