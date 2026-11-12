/* reflection/Descriptor.h — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation (F:\2013Decompile\exports\decomp\Refl_Descriptor.cpp.txt)
     - ctor 0x15F1B0: layout { vftable, bool isDeprecated(+4), const Descriptor* preferred(+8),
       const Name* name(+12) }; RBXCRASH() on lockedDown; RBXASSERT(!name.empty())
     - dtor 0x44D670
   2013 divergence from 2016: deprecated flag/preferred pointer are direct members
   (2016 wraps them in an Attributes struct and adds scoped_ptr<bool> isReplicable/isOutdated,
   which do NOT exist in the 2013 binary). */

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
				// If this following assertion fails then you need to put your class
				// into FactoryRegistrator::FactoryRegistrator() or somewhere else.
				// Failure of this test is so severe that we want to catch it in production, too.
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
					Attributes result;
					result.isDeprecated = true;
					result.preferred = &preferred;
					return result;
				}
				static Attributes deprecated()
				{
					Attributes result;
					result.isDeprecated = true;
					return result;
				}
			};

			static bool lockedDown;	// after the first instance of a described class is created we cannot modify the reflection database

			const bool isDeprecated;				// +4 (flattened Attributes member in the binary)
			const Descriptor* const preferred;		// +8
			const RBX::Name* name;					// +12 (binary stores Name*, not a reference)

			// STUB: PLAYER 0x00455710
			// STUB: STUDIO 0x004113e0
			Descriptor(const char* name, Attributes attributes)
				:isDeprecated(attributes.isDeprecated)
				,preferred(attributes.preferred)
				,name(RBX::Name::declare(name))
			{
				checkLockedDown();
				RBXASSERT(!this->name->str.empty());
			}

			Descriptor(const RBX::Name* name)
				:isDeprecated(false)
				,preferred(NULL)
				,name(name)
			{
				checkLockedDown();
				RBXASSERT(!this->name->str.empty());
			}

			// STUB: PLAYER 0x00455810
			// STUB: STUDIO 0x00407c00
			virtual ~Descriptor() {}
		};
	}
}
