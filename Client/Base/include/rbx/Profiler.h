// Base/include/rbx/Profiler.h: October 2013 reconstruction (Project OGRE).
// No October 2013 binary has this profiler: there is no RBX::Profiler symbol, string or MicroProfile
// code in PLAYER, STUDIO, Mac or iOS (2013's profiling is RBX::Profiling and OnScreenProfiler,
// both in App). The header survives only because files imported from the prior tree call it:
// TaskScheduler.Thread.cpp (onThreadCreate/onThreadExit) and DataModel.cpp (RBXPROFILER_SCOPE).
// Base/rbx/Profiler.cpp defines those as no-ops. Kept declarations and macros: the ones in use.
// Dropped (2016-only, no caller): labels, counters, frame/GPU hooks, mouse input, the renderer.
// RBXPROFILER stays defined on Windows so the existing callers compile as before; removing their
// calls is the callers' job, not this header's.
#pragma once

#include <stdint.h>

#include "RbxFormat.h"

#if defined(_WIN32) || defined(__APPLE__) || defined(__ANDROID__)
#define RBXPROFILER
#endif

namespace RBX {
namespace Profiler {

typedef uint64_t Token;

Token getToken(const char* group, const char* name, int color = -1);
uint64_t enterRegion(Token token);
void leaveRegion(Token token, uint64_t enterTimestamp);

void onThreadCreate(const char* name);
void onThreadExit();

// Times the enclosing block.
struct Scope
{
	Token token;
	uint64_t timestamp;

	Scope(Token t) : token(t) { timestamp = enterRegion(token); }
	~Scope() { leaveRegion(token, timestamp); }
};

}
}

#define RBXPROFILER_CAT2(a, b) a##b
#define RBXPROFILER_CAT(a, b) RBXPROFILER_CAT2(a, b)

#if defined(RBXPROFILER)
// One static token per use site, then a Scope bound to it.
#define RBXPROFILER_SCOPE(group, name, ...) \
	static ::RBX::Profiler::Token RBXPROFILER_CAT(proftoken, __LINE__) = ::RBX::Profiler::getToken(group "", name "", ##__VA_ARGS__); \
	::RBX::Profiler::Scope RBXPROFILER_CAT(profscope, __LINE__)(RBXPROFILER_CAT(proftoken, __LINE__))
#else
#define RBXPROFILER_SCOPE(group, name, ...) (void)0
#endif
