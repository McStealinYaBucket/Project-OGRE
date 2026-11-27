// Placeholder so the ClientShared OBJECT library has a source to build (STUDIO only). Delete it once
// the library has real sources (and drop it from CMakeLists.txt).
//
// Shell template stand-in -- NOT original code, carries no marker. STUDIO's /GL shell instantiates
// Stats::Item::Item itself (its FrameRateManagerStatsItem, 0x0043e8a0, sits beside it): the copy all
// callers reach is the shell's LTCG one at STUDIO 0x0043e560 (/GS cookie, x87), not App's. The shell is
// linked first, so instantiating the inline ctor here makes our link resolve it the same way (see
// Client/WindowsClient/placeholder.cpp). Delete it when FrameRateManagerStatsItem is written.

#include "V8DataModel/StatsItem.h"
#include "V8DataModel/DataModel.h"

#include <new>

#include <boost/bind.hpp>
#include <boost/weak_ptr.hpp>

namespace RBX {
	class DataModel;
}

// Wave 18: bind/list1<weak_ptr<DataModel>> are shell-owned in STUDIO: every engine caller (0x006078e9
// ...) reaches the shell's 0x0042b260 / 0x0042e370 (the generator's MATCH_DROP, which this retires).
// Not here: list<rbx::signals::connection>::_Buynode, whose shell copy 0x0049de90 InstancePacketCache's
// ctor calls; instantiating it moves the symbol off the engine fold 0x006d13d0 that Http.h's STUDIO
// TEMPLATE names: +1 (0x0049de90), -4 (0x006d13d0 and three of its callers), measured wave 18.
static void ogreStudioWeakDataModelUse(boost::weak_ptr<RBX::DataModel>)
{
}

// Wave 21: shared_from<DataModel> and shared_from<Instance> are shell-owned in STUDIO: 2013's engine callers
// (filterChangedProperty 0x00638330, the PacketJob ctor 0x00620870, Instance::onDescendantAdded 0x006f11e0,
// ServiceProvider::onChildAdded 0x0076f7b0, ...) reach the shell's folded shared_from body 0x00445440, which
// RunService::start 0x0079af80 also calls as shared_from<RunService>. Without the shell copies our engine
// fold 0x00539ef0 takes those callers (the generator's STUDIO MATCH_DROP, which this retires).
extern "C" void ogreStudioShellStandInUse(void* raw)
{
	new (raw) boost::shared_ptr<RBX::DataModel>(RBX::shared_from(static_cast<RBX::DataModel*>(raw))); // no operator=/dtor
	new (raw) boost::shared_ptr<RBX::Instance>(RBX::shared_from(static_cast<RBX::Instance*>(raw)));   // same body, same copy
	new (raw) RBX::Stats::Item();
	boost::bind(&ogreStudioWeakDataModelUse, *static_cast<boost::weak_ptr<RBX::DataModel>*>(raw));
}

#pragma comment(linker, "/include:_ogreStudioShellStandInUse")
