/* script/ScriptStats.h -- October 2013 (per-state script-stats holder).
   Evidence: ScriptContext::setCollectScriptStats 0x18B23E (batch599)
     operators-new 0x40 bytes and calls C1 0x1AC40E, then
     shared_ptr<ScriptStats>::reset()s it into SC+280.
   Stats-era revision (2026-10-18, batches 713/716/719/733/757): the
   holder is no longer opaque. getScriptStatsNew 0x18A6AC (batch733)
   walks an internal red-black std::map keyed by the SAME string key
   getScriptStatsNew looks up in the +756 ScriptStatInformation map,
   and reads a node count at the holder's +20 (map root at +12). The
   resume-thread decompile (batch716:503-507, 592-596) calls
   scriptResumeStarted/scriptResumeStopped with getFullName() as the
   key. Meter type + fill semantics are binary-pinned through
   getScriptStatsNew's visible prefix; body-shaped internals (the
   start/stop/resume algorithms) are unrecovered in C:\tmp and land as
   the 2016 reference forms (ScriptStats.cpp, 2016-line 1-59) -- loud
   DELTA in ScriptStats.cpp.
   Binary size pin: GCC-ABI holder object = 0x40 bytes. Our member
   shape (US-ABI std::map + std::stack<string>) is TU-internal
   (only ScriptStats.cpp + ScriptContext.cpp touch it), so offsets are
   not exposed; sizeof drift is documented, same standing class as the
   pre-landing opaque stub. */

#pragma once

#include "rbx/RunningAverage.h"		// ActivityMeter<2> / InvocationMeter<2>
#include <boost/shared_ptr.hpp>
#include <map>
#include <stack>
#include <string>

namespace RBX {

	class ScriptStats
	{
	public:
		struct StatCollection
		{
			boost::shared_ptr<ActivityMeter<2> > activity;
			boost::shared_ptr<InvocationMeter<2> > invocations;
		};
		// Binary: the red-black map inside the 0x40 holder (root +12,
		// node count +20 per batch733:317-319). Member layout is our
		// own (TU-internal, see header note above).
		typedef std::map<std::string, StatCollection> ScriptActivityMeterMap;

	protected:
		ScriptActivityMeterMap scriptActivityMap;

		// 0x40-byte holder minus the meter map leaves room for a
		// small stack; the resume bodies need a LIFO (2016 shape).
		//.Binary-evidence-thin: carried, delta-documented.
		std::stack<std::string> scriptStack;

		void stopCollection(const std::string& scriptHash);
		void startCollection(const std::string& scriptHash, bool firstTime);
	public:

		ScriptStats();	// 0x1AC40E (C1)

		void scriptResumeStarted(const std::string& scriptHash);
		void scriptResumeStopped(const std::string& scriptHash);

		const ScriptActivityMeterMap& getScriptActivityMap() const { return scriptActivityMap; }
	};

} // namespace RBX
