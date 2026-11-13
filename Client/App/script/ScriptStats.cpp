/* script/ScriptStats.cpp -- October 2013 (the meter-map TU).
   Home: a dedicated script\ScriptStats.cpp TU in the binary (same
   script-folder group as CoreScript.cpp/StarterScript.cpp, vcxproj-
   wired this era). The C1 ctor 0x1AC40E body plus the four member
   bodies are unrecovered in the C:\tmp dumps:
     - ctor: caller evidence only (batch719:205-208 news 0x40 bytes
       and shared_ptr-reset()s it; batch599 setCollectScriptStats).
     - scriptResumeStarted/scriptResumeStopped: call-site evidence
       only (batch716:503-507 / 592-596, gated on holder-px and
       script-px non-NULL, keyed by getFullName()).
   The bodies below are therefore the 2016 reference forms
   (2016 script\ScriptStats.cpp:1-59), which carry the same meter
   vocabulary the binary-pinned getScriptStatsNew reader walks
   (ActivityMeter averageValue, InvocationMeter getTotalValuePerSecond).
   LOUD DELTA: ctor + these four bodies are conventional 2016-shape
   ports, not binary-faithful reconstructions. */
#include "stdafx.h"
#include "script/ScriptStats.h"
namespace RBX
{
	// mac: 0x1AC40E: C1 no-op (the holder retains its default meters state until the resume probes add map entries).
	ScriptStats::ScriptStats()
	{}
	// 2016-shaped (body unrecovered; see header).
	void ScriptStats::stopCollection(const std::string& scriptHash)
	{
		ScriptActivityMeterMap::iterator iter = scriptActivityMap.find(scriptHash);
		if (iter != scriptActivityMap.end()) {
			iter->second.activity->decrement();
		}
		else{
			//We lost this sample somehow
		}
	}
	// 2016-shaped (body unrecovered; see header).
	void ScriptStats::startCollection(const std::string& scriptHash, bool firstTime)
	{
		ScriptActivityMeterMap::iterator iter = scriptActivityMap.find(scriptHash);
		if (iter != scriptActivityMap.end()) {
			iter->second.activity->increment();
			if (firstTime)
				iter->second.invocations->increment();
		}
		else{
			StatCollection stats;
			stats.activity = boost::shared_ptr<ActivityMeter<2> >(new ActivityMeter<2>());
			stats.invocations = boost::shared_ptr<InvocationMeter<2> >(new InvocationMeter<2>());
			std::pair<ScriptActivityMeterMap::iterator, bool> newEntry =
				scriptActivityMap.insert(ScriptActivityMeterMap::value_type(scriptHash, stats));
			if (newEntry.second){
				newEntry.first->second.activity->increment();
				if (firstTime)
					newEntry.first->second.invocations->increment();
			}
		}
	}
	// 2016-shaped (body unrecovered; call gates binary-pinned batch716:503-507 -- only when the holder and script px are live, keyed by getFullName()).
	void ScriptStats::scriptResumeStarted(const std::string& scriptHash)
	{
		if (!scriptStack.empty()){
			stopCollection(scriptStack.top());
		}
		startCollection(scriptHash, true);
		scriptStack.push(scriptHash);
	}
	// 2016-shaped (body unrecovered; call gates binary-pinned batch716:592-596, the post-lua_resume/continuation slot).
	void ScriptStats::scriptResumeStopped(const std::string& scriptHash)
	{
		RBXASSERT(!scriptStack.empty());
		RBXASSERT(scriptStack.top() == scriptHash);
		stopCollection(scriptHash);
		scriptStack.pop();
		if (!scriptStack.empty()){
			startCollection(scriptStack.top(), false);
		}
	}

} // namespace RBX
