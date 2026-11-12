/* Util/InterpolatedCFrame.h -- October 2013 (network-interpolation history).
   Evidence: ctor + clearHistory 0x20720A (ported, verified) / setValue 0x2072CC /
     computeValue 0x207BAE / interpolate 0x2075DE / computeSampleTargetTime 0x207AD4 /
     setRenderedFrame 1-arg 0x207D6E + 2-arg 0x207DD4 / getSampleInterval 0x207E46 /
     addInterpolationSample 0x447962 (PartInstance side); asserts leak
     Client/App/util/InterpolatedCFrame.cpp (:81 numFrames==NUM_HISTORY,
     :185/:195 alpha<=1.0, :264 now>=prevFrame.localTime).
   Layout (ctor 0x455788 + cross-refs; 128 B, PartInstance+184): renderedFrame +0;
     prevLocal +48; prevRemote +56; accum +64; flag +72; needs-compute latch +73;
     lastDelta +80 (stores a Time-Interval result, 8 B); rate +88 (1.0);
     FrameInfo ring +96..127, capacity 5 = NUM_HISTORY (FrameInfo 64 B =
     CoordinateFrame + Time local + RemoteTime remote).
   LATCH ALIAS (verified): ICF+73 == PartInstance+257 == isInContinousMotion's
     flag. setValue/interpolate set it; computeValue clears at entry (callers
     observe it via PartInstance::isInContinousMotion). Our embedding differs,
     so setValue writes part->continousMotion directly and computeValue syncs
     it at exit (documented; observably identical to the shared-memory binary).
   DELTAS (all marked): fixed FrameInfo[5] ring instead of boost's
     circular_buffer_space_optimized (absent from boost 1.53 + cross-CRT heap
     rule, session 72; identical observable on all paths); lastDelta kept as
     Time::Interval (binary stores the 8 B operator- result into the slot);
     computeValue takes (out, part) — binary's part arg is ignored, ours syncs
     the latch through it (NULL skips sync, test-only); evict subtracts the
     dropped leading remote-interval (verified via the accum==last-first
     invariant + uniform-spacing check against getSampleInterval).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/util/InterpolatedCFrame.cpp */

#pragma once

#include "G3D/CoordinateFrame.h"
#include "rbx/rbxtime.h"

namespace RBX {

	class PartInstance;
	class IMoving;

	class InterpolatedCFrame
	{
		// PartInstance::getLastComputedRenderingCoordinateFrame reads the
		// private renderedFrame directly in the binary (0x448169-0x44818B
		// copies the rotation/translation field stores raw).  GCC = code-
		// level member copying, friendship-free; MSVC needs the friendship
		// to compile the landed field read (name-invisible in emitted code;
		// mirrors PartInstance.h's reciprocal `friend class InterpolatedCFrame;`).
		friend class PartInstance;

	public:
		struct FrameInfo
		{
			G3D::CoordinateFrame frame;
			Time localTime;
			RemoteTime remoteTime;

			FrameInfo() {}
			FrameInfo(const G3D::CoordinateFrame& f, const Time& local, const RemoteTime& remote)
				: frame(f), localTime(local), remoteTime(remote) {}
		};

		static const int NUM_HISTORY = 5;

		InterpolatedCFrame();

		// mac: 0x20720A: collapse last into rendered (iff nonempty), clear, zero accum.
		void clearHistory();
		// mac: 0x2072CC: latch=1, notifyMoved(part+96), push/overwrite/seed (see cpp).
		void setValue(PartInstance* part, const G3D::CoordinateFrame& cf, const RemoteTime& remoteTime);
		// mac: 0x207BAE: latch=0; size<=1 or stale-target copies rendered; else scan +
		// rate adapt + interpolate; out=rendered. part syncs the latch (may be NULL).
		void computeValue(G3D::CoordinateFrame& out, PartInstance* part);
		// mac: 0x207AD4: now - (0.5*accum + lastDelta), full-buffer drift compensation.
		Time computeSampleTargetTime(const Time& now) const;
		// mac: 0x2075DE: two-stage lerp blend; latch=1; prevLocal=now; prevRemote=target.
		void interpolate(const Time& now, const Time& target, int upper);
		// mac: 0x207D6E: rendered=cf, prevLocal=now. 0x207DD4: additionally prevRemote=rt.
		void setRenderedFrame(const G3D::CoordinateFrame& cf);
		void setRenderedFrame(const G3D::CoordinateFrame& cf, const RemoteTime& remoteTime);
		// mac: 0x207E46: (float)(accum / size).
		float getSampleInterval() const;

	private:
		const FrameInfo& frameAt(int logical) const;
		FrameInfo& frameAt(int logical);

		G3D::CoordinateFrame renderedFrame;
		Time prevLocal;
		Time prevRemote;
		double sampleAccum;
		unsigned char flagA;
		bool needsCompute;
		Time::Interval lastDelta;
		double rateAdjust;
		// Fixed ring (see header note); logical 0..size-1 oldest->newest.
		FrameInfo history[NUM_HISTORY];
		int historySize;
		int historyBack;
	};

} // namespace RBX
