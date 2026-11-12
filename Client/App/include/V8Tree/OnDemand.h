/* V8Tree/OnDemand.h -- October 2013 (lazy per-instance state blocks).
   Evidence: OnDemandInstance C2/D2 (pool 20, vtable 0xF50520) /
     OnDemandPVInstance C2 0x48E8B8 (pool 24) / OnDemandPartInstance C2 0x442DBC
     + D2 0x44CCA8 (pool 200); initOnDemand virtual slot 3 with exactly 3
     overrides (Instance/PVInstance/PartInstance); Instance::onDemandRead
     0x543F2A (this+0x4C, NULL until first Write) / onDemandWrite 0x543F36
     (virtual create + line-1090 assert); member table (signals/getters/firers)
     per IDA cross-refs; asserts reference Client/App/v8tree/Instance.cpp.
   HIERARCHY mirrors the instances: OnDemandPartInstance : OnDemandPVInstance
     : OnDemandInstance (C2 delegation 0x442DBC->0x48E8B8->0x68EFF0).
     OnDemandInstance itself lives in V8Tree/Instance.h (pre-existing port
     with Allocator base + child/descendant signals); this header adds only the
     PV (+legacyOffset) and Part (+sleep/clump/touch layout) variants.
   DELTAS (all marked): +32/+36 signal NAMES by firer (TODO-verify); +76
     ComputeProp modeled as opaque bytes (TODO); +188 global + gaps skipped;
     touchedSlotCount is plain int (binary: lock xadd — MT atomicity TODO);
     legacyOffset starts NULL (binary C2 zeros; eager-new path unmapped).
   CRT rule (session 72): blocks are new'd in initOnDemand (lib) and delete'd
     in ~Instance (lib) — same-side; signals connect/disconnect same-side.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8tree/OnDemand.h (path inferred from Instance.cpp evidence) */

#pragma once

#include "V8Tree/Instance.h"
#include "G3D/CoordinateFrame.h"

namespace G3D { class CoordinateFrame; }

namespace RBX {

	class PartInstance;
	class TouchTransmitter;

	typedef rbx::signal<void(shared_ptr<Instance>)> TouchedSignal;

	class OnDemandPVInstance : public OnDemandInstance
	{
	public:
		OnDemandPVInstance()
			: legacyOffset(NULL)
		{}
		virtual ~OnDemandPVInstance()
		{
			delete legacyOffset;
		}

		// +20 (heap 48B; eager-new path unmapped — see header note).
		G3D::CoordinateFrame* legacyOffset;
	};

	class OnDemandPartInstance : public OnDemandPVInstance
	{
	public:
		explicit OnDemandPartInstance(PartInstance* owner);
		virtual ~OnDemandPartInstance() {}

		// +24 (fired onSleepingChanged 0x444D40).
		rbx::signal<void(bool)> sleepingSignal;
		// +28 (fired onClumpChanged 0x444C55).
		rbx::signal<void()> clumpSignal;
		// +32 (fired setCoordinateFrameChanged 0x447C06; NAME TODO-verify).
		rbx::signal<void()> coordinateFrameChangedSignal;
		// +36 (fired setPartSizeUi 0x442739; NAME TODO-verify).
		rbx::signal<void()> partSizeChangedSignal;
		// +40 (stored addTouchTransmitter 0x442B33; assert names it).
		TouchTransmitter* touchTransmitter;
		// +44 (lock xadd incr/decr; MT atomicity TODO — see header note).
		int touchedSlotCount;
		// +48 (getter 0x44B6AC; fired reportTouch 0x44A928).
		TouchedSignal touchedSignal;
		// +52 (ctor arg).
		PartInstance* ownerA;
		// +56 (getter 0x44B6F5; fired reportUntouch 0x4455D8).
		TouchedSignal touchedEndedSignal;
		// +60 (ctor arg; second back-ptr role TODO-verify).
		PartInstance* ownerB;
		// +64 (deprecated, still fired physicsStep 0x5162E1).
		TouchedSignal stoppedTouchingSignal;
		// +68 (getter 0x44B729; fired fireOutfitChanged 0x44A8DE).
		rbx::signal<void()> outfitChangedSignal;
		// +72 (getter 0x44B66D; fired physicsStep 0x516285).
		TouchedSignal localSimulationTouchedSignal;
		// +76 (ComputeProp<Part,OnDemand> storage, 16B; getPart returns it).
		unsigned char computePropState[16];
		// +132 (cached rendering CF, 48B; rewritten by calcRenderingCoordinateFrame).
		G3D::CoordinateFrame renderingFrame;
		// +180 (init 1; set by ~10 setters).
		bool frameDirty;
		// +184 (self).
		OnDemandPartInstance* selfRef;
		// +188..+196 unmapped (binary reads a stream-removing byte at
		// +196: getIsCurrentlyStreamRemovingPart 0x44562A, batch284).
		char reserved188[8];
		bool streamRemoving;  // +196 (C2 false).
	};

} // namespace RBX
