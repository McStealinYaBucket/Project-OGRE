/* Base/rbx/ProcessPerfCounter.cpp: October 2013 reconstruction (Project OGRE).
   Evidence: this path is a 2013 __FILE__; its 14 asserts pin the lines in init (104 for the object
   enumeration, 146-201 for the counter paths). PLAYER bodies: GetData2(long) 0x009b84e0,
   GetData2(double) 0x009b8520, GetProcessCores 0x009b8560, init 0x009b85e0, then the default
   constructor 0x009b8ea0, which 2013 links after init. init finds the PDH "Process" instance whose
   "ID Process" equals pid (instance names repeat, so each gets its #index), then adds the
   per-process counters, falling back to "Working Set" where "Working Set - Private" is missing
   (Windows XP). The Mac build has no counterpart, so the non-Windows branches are dropped. */
#include "rbx/ProcessPerfCounter.h"

#include "rbx/Debug.h"
#include "rbx/RbxDbgInfo.h"
#include <map>

#if defined(_WIN32)
#include "PdhMsg.h"
#pragma comment(lib, "pdh.lib")

PerfCounter::PerfCounter()
{
	PDH_STATUS opened = PdhOpenQuery(NULL, 0, &hQuery);
}

CProcessPerfCounter::CProcessPerfCounter(int pid)
{
	init(pid);
}

void PerfCounter::CollectData()
{
	PDH_STATUS collected = PdhCollectQueryData(hQuery);
}

// FUNCTION: PLAYER 0x009b84e0
// FUNCTION: STUDIO 0x00a6d740
void PerfCounter::GetData2(HCOUNTER counter, long& result)
{
	PDH_FMT_COUNTERVALUE value = {0};
	PDH_STATUS formatted = PdhGetFormattedCounterValue(counter, PDH_FMT_LONG, NULL, &value);
	result = value.longValue;
}

// FUNCTION: PLAYER 0x009b8520
// FUNCTION: STUDIO 0x00a6d780
void PerfCounter::GetData2(HCOUNTER counter, double& result)
{
	PDH_FMT_COUNTERVALUE value = {0};
	PDH_STATUS formatted = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value);
	result = value.doubleValue;
}

// FUNCTION: PLAYER 0x009b8560
// FUNCTION: STUDIO 0x00a6d7c0
double CProcessPerfCounter::GetProcessCores()
{
	double machineLoad; // percent of all processors in use
	GetData2(totalProcessorTimeCounter, machineLoad);
	double processLoad; // percent this process uses
	GetData2(processorTimeCounter, processLoad);
	return machineLoad / 100.0 * processLoad / 100.0 * (double)numCores;
}

// The counter path for the counter named in `where`, and adding it to this object's query.
#define MAKE_PATH() PdhMakeCounterPath(&where, path, &pathLength, 0)
#define ADD_COUNTER(handle) PdhAddCounter(hQuery, path, 0, &handle)
#define NEXT_COUNTER(text) where.szCounterName = text; pathLength = 1024

// FUNCTION: PLAYER 0x009b85e0
// FUNCTION: STUDIO 0x00a6d840
void CProcessPerfCounter::init(int pid)
{
	::GetSystemInfo(&systemInfo);
	numCores = systemInfo.dwNumberOfProcessors;
	RBX::RbxDbgInfo::s_instance.NumCores = numCores;

	totalProcessorTimeCounter = 0;
	processorTimeCounter = 0;
	privateBytesCounter = 0;
	pageFaultsPerSecondCounter = 0;
	pageFileBytesCounter = 0;
	virtualBytesCounter = 0;
	workingSetPrivateCounter = 0;
	// What init leaves in the query, all for this process's PDH instance:
	//   processorTimeCounter        "% Processor Time" (share of one processor, can pass 100)
	//   elapsedTimeCounter          "Elapsed Time" since the process started, seconds
	//   privateBytesCounter         "Private Bytes" committed to this process alone
	//   pageFaultsPerSecondCounter  "Page Faults/sec"
	//   pageFileBytesCounter        "Page File Bytes" in use by the process
	//   virtualBytesCounter         "Virtual Bytes" of reserved address space
	//   workingSetPrivateCounter    "Working Set - Private" (Vista on), else "Working Set"
	//   totalProcessorTimeCounter   \\Processor(_Total)\\% Processor Time, machine-wide
	// The instance is found by enumerating "Process" and reading each instance's "ID Process" in a
	// throwaway query until it equals pid. PDH names a second copy of the same exe "name#1", so the
	// enumeration counts how often each name has been seen and passes that as dwInstanceIndex.
	// GetProcessCores turns the two processor counters into "cores in use" with numCores.


	TCHAR* names = new TCHAR[100000];
	TCHAR* instance;
	{
		DWORD namesLength = 100000;
		DWORD noCounters = 0;
		PDH_STATUS status = PdhEnumObjectItems(NULL, NULL, "Process", NULL, &noCounters, names, &namesLength, PERF_DETAIL_EXPERT, 0);
		RBXASSERT(SUCCEEDED(status) || status == PDH_MORE_DATA);

		std::map<std::string, int> seen;
		// names is a list of strings ended by an empty one
		instance = names;
		while (instance[0] != 0)
		{
			std::string key(instance);
			seen.insert(std::pair<std::string, int>(key, 0));

			TCHAR idPath[1024];
			DWORD idPathLength = 1024;
			PDH_COUNTER_PATH_ELEMENTS where = {0};
			CQuery probe;
			PDH_STATUS pdhResult = PdhOpenQuery(NULL, 0, &probe);
			where.szObjectName = "Process";
			where.szCounterName = "ID Process";
			where.szInstanceName = instance;
			where.dwInstanceIndex = seen[key]++;
			pdhResult = PdhMakeCounterPath(&where, idPath, &idPathLength, 0);

			CQuery idCounter;
			pdhResult = PdhAddCounter(probe, idPath, 0, &idCounter);
			pdhResult = PdhCollectQueryData(probe);

			PDH_FMT_COUNTERVALUE id = {0};
			pdhResult = PdhGetFormattedCounterValue(idCounter, PDH_FMT_LONG, NULL, &id);
			if (id.longValue == pid)
				break;

			instance += strlen(instance) + 1;
		}
	}


	TCHAR path[1024];
	DWORD pathLength = 1024;
	PDH_COUNTER_PATH_ELEMENTS where = {0};
	where.szObjectName = "Process";
	where.szCounterName = "% Processor Time";
	where.szInstanceName = instance;
	PDH_STATUS pdhResult = MAKE_PATH();
	RBXASSERT(SUCCEEDED(pdhResult));
	pdhResult = ADD_COUNTER(processorTimeCounter);
	RBXASSERT(SUCCEEDED(pdhResult));


	NEXT_COUNTER("Elapsed Time");
	pdhResult = MAKE_PATH();
	RBXASSERT(SUCCEEDED(pdhResult));
	pdhResult = ADD_COUNTER(elapsedTimeCounter);
	RBXASSERT(SUCCEEDED(pdhResult));


	NEXT_COUNTER("Private Bytes");
	pdhResult = MAKE_PATH();
	RBXASSERT(SUCCEEDED(pdhResult));
	pdhResult = ADD_COUNTER(privateBytesCounter);
	RBXASSERT(SUCCEEDED(pdhResult));


	NEXT_COUNTER("Page Faults/sec");
	pdhResult = MAKE_PATH();
	RBXASSERT(SUCCEEDED(pdhResult));
	pdhResult = ADD_COUNTER(pageFaultsPerSecondCounter);
	RBXASSERT(SUCCEEDED(pdhResult));


	NEXT_COUNTER("Page File Bytes");
	pdhResult = MAKE_PATH();
	RBXASSERT(SUCCEEDED(pdhResult));
	pdhResult = ADD_COUNTER(pageFileBytesCounter);
	RBXASSERT(SUCCEEDED(pdhResult));


	NEXT_COUNTER("Virtual Bytes");
	pdhResult = MAKE_PATH();
	RBXASSERT(SUCCEEDED(pdhResult));
	pdhResult = ADD_COUNTER(virtualBytesCounter);
	RBXASSERT(SUCCEEDED(pdhResult));

	NEXT_COUNTER("Working Set - Private");
	pdhResult = MAKE_PATH();
	pdhResult = ADD_COUNTER(workingSetPrivateCounter);
	if (pdhResult != 0)
	{
		// no private working set counter (XP): use the plain one, unchecked
		NEXT_COUNTER("Working Set");
		pdhResult = MAKE_PATH();
		pdhResult = ADD_COUNTER(workingSetPrivateCounter);
	}

	// (spacing: keeps the asserts below on their 2013 line numbers)



	pdhResult = PdhAddCounter(hQuery, "\\Processor(_Total)\\% Processor Time", 0, &totalProcessorTimeCounter);
	RBXASSERT(SUCCEEDED(pdhResult));

	delete[] names;
}

#undef MAKE_PATH
#undef ADD_COUNTER
#undef NEXT_COUNTER

// FUNCTION: PLAYER 0x009b8ea0
// FUNCTION: STUDIO 0x00a6e100
CProcessPerfCounter::CProcessPerfCounter()
{
	init(::GetCurrentProcessId());
}

#endif
