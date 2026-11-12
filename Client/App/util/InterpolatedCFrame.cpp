/* App/util/InterpolatedCFrame.cpp -- October 2013 (full sample algorithm).
   See Util/InterpolatedCFrame.h for evidence + deltas. Bodies per decomp
   (member offsets + assert file:lines in the header). Layering note: this TU
   includes V8DataModel/PartInstance.h solely for the +73/+257 latch sync
   (PartInstance grants friendship); the algorithm itself touches no Part state.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/util/InterpolatedCFrame.cpp */

#include "stdafx.h"

#include "Util/InterpolatedCFrame.h"
#include "G3D/Quat.h"
#include "G3D/Matrix3.h"
#include "V8World/IMoving.h"
#include "V8DataModel/PartInstance.h"
#include "rbx/Debug.h"

namespace RBX {

	InterpolatedCFrame::InterpolatedCFrame()
		: renderedFrame()
		, prevLocal()
		, prevRemote()
		, sampleAccum(0.0)
		, flagA(0)
		, needsCompute(false)
		, lastDelta()
		, rateAdjust(1.0)
		, historySize(0)
		, historyBack(0)
	{
	}

	const InterpolatedCFrame::FrameInfo& InterpolatedCFrame::frameAt(int logical) const
	{
		RBXASSERT(logical >= 0 && logical < historySize);
		int physical = (historyBack - (historySize - 1) + logical + 2 * NUM_HISTORY) % NUM_HISTORY;
		return history[physical];
	}

	InterpolatedCFrame::FrameInfo& InterpolatedCFrame::frameAt(int logical)
	{
		RBXASSERT(logical >= 0 && logical < historySize);
		int physical = (historyBack - (historySize - 1) + logical + 2 * NUM_HISTORY) % NUM_HISTORY;
		return history[physical];
	}

	void InterpolatedCFrame::clearHistory()
	{
		if (historySize > 0)
		{
			const FrameInfo& last = frameAt(historySize - 1);
			renderedFrame = last.frame;
			prevLocal = last.localTime;
			prevRemote = last.remoteTime;
			historySize = 0;
		}
		sampleAccum = 0.0;
		flagA = 0;
		lastDelta = Time::Interval();
		rateAdjust = 1.0;
	}

	// STUB: PLAYER 0x008dc8c0
	// STUB: STUDIO 0x009c96c0
	void InterpolatedCFrame::setValue(PartInstance* part, const G3D::CoordinateFrame& cf, const RemoteTime& remoteTime)
	{
		needsCompute = true;
		part->continousMotion = true;  // +73/+257 latch alias (see header).
		static_cast<IMoving*>(part)->notifyMoved();
		Time now = Time::nowFast();
		lastDelta = now - remoteTime;
		if (historySize > 0)
		{
			const FrameInfo& last = frameAt(historySize - 1);
			double dt = (remoteTime - last.remoteTime).seconds();
			if (dt < 0.0)
				return;  // Drop out-of-order samples.
			if (dt == 0.0)
			{
				FrameInfo& slot = frameAt(historySize - 1);
				slot.frame = cf;
				slot.localTime = now;
				return;  // Same remote: overwrite in place (no growth).
			}
		}
		if (historySize == NUM_HISTORY)
		{
			// Evicting while full: capacity must still be NUM_HISTORY (:81).
			RBXASSERT(historySize == NUM_HISTORY);
			const FrameInfo& evicted = frameAt(0);
			const FrameInfo& next = frameAt(1);
			sampleAccum -= (next.remoteTime - evicted.remoteTime).seconds();
		}
		FrameInfo incoming(cf, now, remoteTime);
		if (historySize == 0)
			historyBack = 0;
		else
			historyBack = (historyBack + 1) % NUM_HISTORY;
		history[historyBack] = incoming;
		if (historySize < NUM_HISTORY)
			++historySize;
		if (historySize < 2)
		{
			// Seed rule: first sample initializes the rendered state.
			prevRemote = remoteTime;
			prevLocal = now;
			renderedFrame = cf;
		}
		else
		{
			const FrameInfo& newest = frameAt(historySize - 1);
			const FrameInfo& prev = frameAt(historySize - 2);
			sampleAccum += (newest.remoteTime - prev.remoteTime).seconds();
		}
	}

	// STUB: PLAYER 0x008dc230
	// STUB: STUDIO 0x009c9030
	void InterpolatedCFrame::computeValue(G3D::CoordinateFrame& out, PartInstance* part)
	{
		Time now = Time::nowFast();
		RBXASSERT(now >= prevLocal);  // :264 (prevFrame.localTime).
		needsCompute = false;
		if (historySize <= 1)
		{
			out = renderedFrame;
			if (part != NULL)
				part->continousMotion = false;
			return;
		}
		Time target = computeSampleTargetTime(now);
		if (prevRemote > target)
		{
			out = renderedFrame;
			if (part != NULL)
				part->continousMotion = false;
			return;
		}
		int upper = historySize;
		for (int i = 0; i < historySize; ++i)
		{
			if (frameAt(i).remoteTime > target)
			{
				upper = i;
				break;
			}
		}
		if (upper == historySize)
		{
			rateAdjust *= 0.9;
			prevLocal = now;
			out = renderedFrame;
			if (part != NULL)
				part->continousMotion = false;
			return;
		}
		if (upper == 0)
			rateAdjust *= 0.5;
		else if (rateAdjust < 1000.0)
			rateAdjust *= 1.1;
		interpolate(now, target, upper);
		out = renderedFrame;
		if (part != NULL)
			part->continousMotion = needsCompute;
	}

	Time InterpolatedCFrame::computeSampleTargetTime(const Time& now) const
	{
		Time r = now - (Time::Interval::from_seconds(0.5 * sampleAccum) + lastDelta);
		if (historySize == NUM_HISTORY)
		{
			const Time& firstRemote = frameAt(0).remoteTime;
			const Time& lastRemote = frameAt(historySize - 1).remoteTime;
			if ((lastRemote - prevRemote).seconds() + sampleAccum >= 0.0 &&
				(prevRemote - firstRemote).seconds() + sampleAccum >= 0.0)
			{
				Time::Interval v8 = now - prevLocal;
				double v6 = sampleAccum * rateAdjust;
				double base = (prevRemote - Time()).seconds();
				double t = ((r - prevRemote).seconds() + v6) / (v6 / v8.seconds() + 1.0) + base;
				r = Time() + Time::Interval::from_seconds(t);
			}
		}
		return r;
	}

	// Exact mirror of G3D::CoordinateFrame::lerp (G3Dcpp/CoordinateFrame.cpp
	// is NOT linked (link-debt decision); composed verbatim from linked pieces
	// + Quat.cpp: slerp rotation, linear translation, 0/1 fast paths.
	static G3D::CoordinateFrame lerpCF(const G3D::CoordinateFrame& a, const G3D::CoordinateFrame& b, float alpha)
	{
		if (alpha == 1.0f)
			return b;
		if (alpha == 0.0f)
			return a;
		const G3D::Quat q1(a.rotation);
		const G3D::Quat q2(b.rotation);
		return G3D::CoordinateFrame(q1.slerp(q2, alpha).toRotationMatrix(),
			a.translation * (1.0f - alpha) + b.translation * alpha);
	}

	// STUB: PLAYER 0x008dbed0
	// STUB: STUDIO 0x009c8cd0
	void InterpolatedCFrame::interpolate(const Time& now, const Time& target, int upper)
	{
		const FrameInfo& hi = frameAt(upper);
		float alpha = (float)((target - prevRemote).seconds() / (hi.remoteTime - prevRemote).seconds());
		RBXASSERT(alpha <= 1.0f);  // :185
		G3D::CoordinateFrame stage1 = lerpCF(renderedFrame, hi.frame, alpha);
		if (upper != 0 && upper < historySize - 1)
		{
			const FrameInfo& last = frameAt(historySize - 1);
			float alpha2 = (float)((target - prevRemote).seconds() / (last.remoteTime - prevRemote).seconds());
			RBXASSERT(alpha2 <= 1.0f);  // :195
			G3D::CoordinateFrame stage2 = lerpCF(renderedFrame, last.frame, alpha2);
			const FrameInfo& lo = frameAt(upper - 1);
			float beta = (float)((target - lo.remoteTime).seconds() / (hi.remoteTime - lo.remoteTime).seconds());
			stage1 = lerpCF(stage1, stage2, beta);
		}
		renderedFrame = stage1;
		needsCompute = true;
		prevLocal = now;
		prevRemote = target;
	}

	void InterpolatedCFrame::setRenderedFrame(const G3D::CoordinateFrame& cf)
	{
		renderedFrame = cf;
		prevLocal = Time::nowFast();
	}

	void InterpolatedCFrame::setRenderedFrame(const G3D::CoordinateFrame& cf, const RemoteTime& remoteTime)
	{
		renderedFrame = cf;
		prevLocal = Time::nowFast();
		prevRemote = remoteTime;
	}

	float InterpolatedCFrame::getSampleInterval() const
	{
		// Binary-identical (fld return; undefined for size 0, same as binary).
		return (float)(sampleAccum / (double)historySize);
	}

} // namespace RBX
