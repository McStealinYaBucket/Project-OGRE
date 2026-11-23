// Base/include/rbx/ProcessPerfCounter.h: October 2013 reconstruction (Project OGRE).
// Evidence: RTTI sp_counted_impl_p<CProcessPerfCounter> (PLAYER), i.e. a ScopedSingleton held in
// a shared_ptr; Base\rbx\ProcessPerfCounter.cpp is a 2013 __FILE__ with GetData2 (long, double),
// GetProcessCores, init and the default constructor at PLAYER 0x009b84e0-0x009b8ea0. The member
// order below is the one init's stores walk. Windows only: neither Mac nor iOS has these symbols,
// so 2016's Durango (Xbox) guard is dropped.
#pragma once

#include "rbx/boost.hpp"
#include "util/ScopedSingleton.h"

#if defined(_WIN32)
#include <pdh.h>

// Owns a PDH query handle and closes it on destruction. &q yields the slot PdhOpenQuery and
// PdhAddCounter write into.
class CQuery
{
	HQUERY handle;
public:
	CQuery(HQUERY h) : handle(h) {}
	CQuery() : handle(0) {}
	~CQuery() { PdhCloseQuery(handle); }

	HQUERY* operator&() { return &handle; }
	operator HQUERY() const { return handle; }
};

// Base for PDH readers: one query that owns every counter added to it. Usage pattern: add counters
// to hQuery once, then per sample call CollectData() and read each counter with GetData2. PDH
// rate counters (processor time, faults per second) need two collections before they mean
// anything, so the first reading after construction is zero.
class PerfCounter
{
protected:
	PerfCounter(); // opens hQuery
	CQuery hQuery;
	// Formatted value of one counter from the last CollectData.
	static void GetData2(HCOUNTER counter, long& result);
	static void GetData2(HCOUNTER counter, double& result);
public:
	void CollectData();
};

// The PDH counters of one process (the current one by default).
class CProcessPerfCounter : public PerfCounter, public RBX::ScopedSingleton<CProcessPerfCounter>
{
public:
	CProcessPerfCounter();
	CProcessPerfCounter(int pid);

	// Processor cores the process keeps busy, from the process and machine processor-time counters.
	double GetProcessCores();

	// Latest values (call CollectData first); each reads one counter through GetData2.
#define OGRE_PERF_GETTER(Type, Getter, counter) \
	Type Getter() { Type v; PerfCounter::GetData2(counter, v); return v; }
	OGRE_PERF_GETTER(double, GetElapsedTime, elapsedTimeCounter)
	OGRE_PERF_GETTER(long, GetTotalProcessorTime, totalProcessorTimeCounter)
	OGRE_PERF_GETTER(long, GetProcessorTime, processorTimeCounter)
	OGRE_PERF_GETTER(long, GetPrivateBytes, privateBytesCounter)
	OGRE_PERF_GETTER(long, GetPageFaultsPerSecond, pageFaultsPerSecondCounter)
	OGRE_PERF_GETTER(long, GetPageFileBytes, pageFileBytesCounter)
	OGRE_PERF_GETTER(long, GetVirtualBytes, virtualBytesCounter)
	OGRE_PERF_GETTER(long, GetPrivateWorkingSetBytes, workingSetPrivateCounter)
#undef OGRE_PERF_GETTER

private:
	unsigned int numCores;
	SYSTEM_INFO systemInfo; // filled by init; only dwNumberOfProcessors is used
	// Counter handles in hQuery; init zeroes all but elapsedTimeCounter before adding them.
	HCOUNTER elapsedTimeCounter;
	HCOUNTER totalProcessorTimeCounter;
	HCOUNTER processorTimeCounter;
	HCOUNTER privateBytesCounter;
	HCOUNTER pageFaultsPerSecondCounter;
	HCOUNTER pageFileBytesCounter;
	HCOUNTER virtualBytesCounter;
	HCOUNTER workingSetPrivateCounter;

	void init(int pid);
};

#endif
