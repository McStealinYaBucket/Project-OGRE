/* V8DataModel/DebrisService.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<DebrisService> from DataModel::initializeContents
     0x2D3336 (created in initializeContents.); "DebrisService" string in both 2013 PEs.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/DebrisService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include <queue>

namespace RBX {

	extern const char* const sDebrisService;

	class TimerService;

	// 2013 layout (PLAYER ctor 0x008455c0, setMaxItems 0x00844e90): RTTI functionality 27 (the
	// default, not RUNTIME_LOCAL), Service @0x60, queue @0x64, maxItems @0x84 (1000),
	// legacyMaxItems @0x88, timer @0x8c. Same members and order as 2016.
	// VTABLE: PLAYER 0x00b78e44 RBX::Debugable
	// VTABLE: STUDIO 0x00c4c57c RBX::Debugable
	// SIZE 0x94
	class DebrisService
		: public DescribedNonCreatable<DebrisService, Instance, sDebrisService>
		, public Service
	{
	private:
		typedef DescribedNonCreatable<DebrisService, Instance, sDebrisService> Super;
		std::queue<weak_ptr<Instance> > queue;		// 0x64
		int maxItems;								// 0x84
		bool legacyMaxItems;						// 0x88
		shared_ptr<TimerService> timer;				// 0x8c
	public:
		DebrisService();
		void setMaxItems(int value);
		int getMaxItems() const { return maxItems; }
	private:
		void cleanup();
	};

} // namespace RBX

// Template, inline and implicit bodies this .obj emits whose 2013 copy has no oracle name: paired
// by identical masked bytes plus string/RTTI/callee operands (tools/unpaired_twins.py).

// TEMPLATE: PLAYER 0x00844f60
// ??$?0V?$bind_t@XP6AXV?$weak_ptr@VDataModel@RBX@@@boost@@@ZV?$list1@V?$value@V?$weak_ptr@VDataModel@RBX@@@boost@@@_bi@boost@@@_bi@2@@_bi@boost@@@?$function1@XPAVDataModel@RBX@@@boost@@QAE@V?$bind_t@XP6AXV?$weak_ptr@VDataModel@RBX@@@boost@@@ZV?$list1@V?$value@V?$weak_ptr@VDataModel@RBX@@@boost@@@_bi@boost@@@_bi@2@@_bi@1@H@Z

