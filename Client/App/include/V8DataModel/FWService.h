/* V8DataModel/FWService.h — October 2013 (flyweight dict owner).
   Evidence: FWService C2 0x6D2BD8 (Instance(nullptr) base + embedded
     FWDictionnary<FWPartInstance> at +96: vtable at +96, unordered_set at
     +100 with 11 buckets, update counter at +124; byte +92 = 1) + D2
     0x6D2D8A (ConcurrencyValidator at +128, set teardown, Instance dtor) +
     getUniqueSharedPtr 0x6D2E24 (aliasing shared_ptr onto the +96 dict with
     service ownership) + isOwnedHolder 0x6D2E8E (holder == this+96).
     Created in DataModel::initializeContents 0x2D3336; "FWService" string in
     both 2013 PEs.
   MSVC member ORDER mirrors the binary (+92 flag, +96 dict, +128 validator);
   binary byte offsets differ (documented throughout the FW framework).
   TODO: what sets +92 / who calls getUniqueSharedPtr (callers pending).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/FWService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"
#include "Util/FeatherweightDict.h"
#include "Util/ConcurrencyValidator.h"

namespace RBX {

	extern const char* const sFWService;

	class FWService
		: public DescribedNonCreatable<FWService, Instance, sFWService, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		FWService();

		// mac: 0x6D2E24: shared_ptr aliasing the owned part dict (keeps the
		// service alive). Requires shared ownership (ServiceProvider::create).
		shared_ptr<FWDictionnaryBase> getUniqueSharedPtr();

		// mac: 0x6D2E8E: membership test for the owned holder.
		bool isOwnedHolder(IFWHolder* holder);

		FWDictionnary<FWPartInstance>& partDict() { return partDictMember; }

	private:
		bool unknown92;											// +92 (C2 sets 1; TODO identity)
		FWDictionnary<FWPartInstance> partDictMember;			// +96 (embedded dict)
		ConcurrencyValidator validator;							// +128
	};

} // namespace RBX
