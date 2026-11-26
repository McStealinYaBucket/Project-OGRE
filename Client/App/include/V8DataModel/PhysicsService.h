/* V8DataModel/PhysicsService.h -- October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<PhysicsService> from DataModel::initializeContents
     0x2D3336 (created in initializeContents.); "PhysicsService" string in both 2013 PEs.
   Functionality + Service base carried from March 2016 conventions.
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/PhysicsService.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sPhysicsService;

	class PhysicsService
		: public DescribedNonCreatable<PhysicsService, Instance, sPhysicsService>
		, public Service
	{
	public:
		PhysicsService() {}

		// PeerStatsItem::update 0x005ba060 reads +0x64 inline (2016: parts.size()).
		int numSenders() { return m_unk0x64; }

	private:
		// ctor 0x00649490: Service::isPublic is +0x60; +0x64..+0x6c are zeroed (2016: Intrusive::Set parts).
		int m_unk0x64;				// 0x64
	};

} // namespace RBX
