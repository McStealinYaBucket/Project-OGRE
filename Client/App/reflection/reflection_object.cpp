/* reflection_object.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
   (F:\2013Decompile\exports\decomp\Impl_reflection_object.cpp.txt)
     - DescribedBase ctor 0x54734C: { vftable, xmlId(+4,+8 zeroed), lockedDown=1,
       descriptor(+12) = &ClassDescriptor::rootDescriptor()::root }
     - DescribedBase dtor 0x547446
     - DescribedBase::isA(std::string) 0x545A64
     - ClassDescriptor members live in ClassDescriptor_ctor.txt evidence
   Layout note: 2013 DescribedBase has NO EventSource base subobject (see Object.h). */

#include "stdafx.h"
#include "decomp.h"

#include "Reflection/Object.h"
#include "Reflection/Event.h"

#include <algorithm>

namespace RBX { namespace Reflection {

	// OGRE vc9: the prior tree used two identical C++11 lambdas; VC9 has none. The 2016 source names
	// this static comparator compare2.
	static bool compare2(const ClassDescriptor* a, const ClassDescriptor* b)
	{
		return *a->name < *b->name;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// ClassDescriptor (0x1577DE ctor evidence — see ClassDescriptor_ctor.txt)
	// Layout: { Descriptor(16), containers +16/+104/+192/+280/+368 (stride 88),
	//           security +456, derivedClasses +460, base +472, bitflags +476 }
	//////////////////////////////////////////////////////////////////////////////////
	// STUB: PLAYER 0x00618440
	// STUB: STUDIO 0x00706330
	ClassDescriptor::ClassDescriptor(ClassDescriptor& base, const char* name, Attributes attributes, Security::Permissions security)
		:Descriptor(name, attributes)
		/* binary 0x1577DE passes (base + 16/104/192/280/368) — the corresponding
		   container subobject of the base ClassDescriptor, or NULL for the root */
		,MemberDescriptorContainer<PropertyDescriptor>(static_cast<MemberDescriptorContainer<PropertyDescriptor>*>(&base))
		,MemberDescriptorContainer<EventDescriptor>(static_cast<MemberDescriptorContainer<EventDescriptor>*>(&base))
		,MemberDescriptorContainer<FunctionDescriptor>(static_cast<MemberDescriptorContainer<FunctionDescriptor>*>(&base))
		,MemberDescriptorContainer<YieldFunctionDescriptor>(static_cast<MemberDescriptorContainer<YieldFunctionDescriptor>*>(&base))
		,MemberDescriptorContainer<CallbackDescriptor>(static_cast<MemberDescriptorContainer<CallbackDescriptor>*>(&base))
		,security(security)
		,base(&base)
		,bReplicateType((attributes.flags >> 1) & 3)
		,bCanXmlWrite((attributes.flags >> 1) & 4 ? 1 : 0)
		,bIsScriptable((attributes.flags >> 1) & 8 ? 1 : 0)
	{
		// register in the base's derivedClasses (binary does a lower_bound insert, 0x1577DE)
		ClassDescriptors& derived = base.derivedClasses;
		ClassDescriptors::iterator iter = std::lower_bound(derived.begin(), derived.end(), this, compare2);
		RBXASSERT(iter == derived.end() || *iter != this);
		derived.insert(iter, this);

		// register in the global allClasses() (sorted by name, call_once-guarded in the binary)
		ClassDescriptors& all = allClasses();
		ClassDescriptors::iterator allIter = std::lower_bound(all.begin(), all.end(), this, compare2);
		all.insert(allIter, this);
	}

	// FUNCTION: PLAYER 0x006182e0
	// FUNCTION: STUDIO 0x007061d0
	ClassDescriptor::ClassDescriptor()	// root ctor 0x1575D6
		:Descriptor("<<<ROOT>>>", Descriptor::Attributes())
		,MemberDescriptorContainer<PropertyDescriptor>(NULL)
		,MemberDescriptorContainer<EventDescriptor>(NULL)
		,MemberDescriptorContainer<FunctionDescriptor>(NULL)
		,MemberDescriptorContainer<YieldFunctionDescriptor>(NULL)
		,MemberDescriptorContainer<CallbackDescriptor>(NULL)
		,security(Security::None)
		,base(NULL)
		,bReplicateType(true)
		,bCanXmlWrite(true)
		,bIsScriptable(true)
	{
	}

	/* rootDescriptor() is inline in Object.h (static local root) — matches binary 0x184430 */

	ClassDescriptor::ClassDescriptors& ClassDescriptor::allClasses()	// 0x157514
	{
		static ClassDescriptors s;
		return s;
	}

	bool ClassDescriptor::isBaseOf(const ClassDescriptor& child) const
	{
		for (const ClassDescriptor* c = &child; c; c = c->base) {
			if (c == this)
				return true;
		}
		return false;
	}

	bool ClassDescriptor::isA(const ClassDescriptor& test) const	// 0x157C0E
	{
		for (const ClassDescriptor* c = this; c; c = c->base) {
			if (c == &test)
				return true;
		}
		return false;
	}

	bool ClassDescriptor::isBaseOf(const char* childName) const
	{
		// TODO(decompile): name-based lookup path (allEnumsNameLookup-style map in the binary)
		for (ClassDescriptors::const_iterator iter = allClasses().begin(); iter != allClasses().end(); ++iter) {
			if ((*iter)->name->str == childName)
				return isBaseOf(**iter);
		}
		return false;
	}

	bool ClassDescriptor::isA(const char* testName) const
	{
		for (ClassDescriptors::const_iterator iter = allClasses().begin(); iter != allClasses().end(); ++iter) {
			if ((*iter)->name->str == testName)
				return isA(**iter);
		}
		return false;
	}

	// FUNCTION: PLAYER 0x00614c30
	// FUNCTION: STUDIO 0x00703540
	bool ClassDescriptor::operator==(const ClassDescriptor& other) const	// 0x157BEA
	{
		return this == &other;
	}

	bool ClassDescriptor::operator!=(const ClassDescriptor& other) const	// 0x157BFC
	{
		return this != &other;
	}

	// Descriptor registration lock flag. TODO: verify 2013 default (false carried
	// from 2016; DescribedBase ctor evidence mentions lockedDown=1 — reconcile).
	bool Descriptor::lockedDown = false;

	void (*MemberDescriptor::memberHidingHook)(MemberDescriptor*, MemberDescriptor*) = 0;

	int ClassDescriptor::count = 0;

	bool MemberDescriptor::isMemberOf(const ClassDescriptor& classDescriptor) const
	{
		const ClassDescriptor* d = &classDescriptor;
		do
		{
			if (*d == this->owner)
				return true;
			d = d->getBase();
		}
		while (*d != ClassDescriptor::rootDescriptor());

		return false;
	}

	bool MemberDescriptor::isMemberOf(const DescribedBase* instance) const
	{
		RBXASSERT(instance != NULL);
		return isMemberOf(instance->getDescriptor());
	}

} } // namespace RBX::Reflection

DECOMP_SIZE_ASSERT(RBX::Reflection::MemberDescriptorContainer<RBX::Reflection::PropertyDescriptor>, 0x60);
DECOMP_SIZE_ASSERT(RBX::Reflection::ClassDescriptor, 0x20c);
