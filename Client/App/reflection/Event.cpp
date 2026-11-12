/* Event.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation
   (F:\2013Decompile\exports\decomp\Impl_Event.cpp.txt)
     - EventDescriptor ctor 0x1134-area (real body in the container declare path;
       matches 2016: MemberDescriptor(classDescriptor, name, "Signals", attributes, security))
     - EventDescriptor::isBroadcast 0x156600 (returns the broadcast flag)
     - EventSource::processRemoteEvent 0x1565B2 / raiseEventInvocation 0x1565D6
       (BOTH ARE STUBS in the Oct 2013 Mach-O — the remote-event raise path was
       not yet wired through EventSource; Instance handles signals directly)
   Layout note: MemberDescriptorContainer uses rbx::trie in the binary
   (depth_exceeded_exception / bad_key at 0x15668C..) for the name lookup. */

#include "stdafx.h"

#include "Reflection/Event.h"
#include "Reflection/Object.h"

using namespace RBX;
using namespace RBX::Reflection;

EventDescriptor::EventDescriptor(ClassDescriptor& classDescriptor, const char* name, Security::Permissions security, Attributes attributes)
	:MemberDescriptor(classDescriptor, name, "Signals", attributes, security)
{
	// TODO: This could be moved up into MemberDescriptor if it were templated...
	classDescriptor.MemberDescriptorContainer<EventDescriptor>::declare(this);
}

std::size_t hash_value(const Event& evt)
{
	std::size_t result = boost::hash<const EventDescriptor*>()(evt.getDescriptor());
	boost::hash_combine(result, evt.getInstance().get());
	return result;
}

RBX::Reflection::RemoteEventCommon::Attributes RBX::Reflection::RemoteEventCommon::Attributes::deprecated(Functionality flags, const MemberDescriptor* preferred)
{
	RemoteEventCommon::Attributes result(flags);
	result.isDeprecated = true;
	result.preferred = preferred;
	return result;
}

/* 2013: both EventSource methods are empty stubs in the binary (0x1565B2 / 0x1565D6) */
// STUB: PLAYER 0x0066edf0
// STUB: STUDIO 0x007412c0
void EventSource::processRemoteEvent(const EventDescriptor& descriptor, const EventArguments& args, const SystemAddress& source)
{
}

void EventSource::raiseEventInvocation(const EventDescriptor& descriptor, const EventArguments& args, const SystemAddress* target)
{
}
