// Base/include/rbx/RbxDbgInfo.h: October 2013 reconstruction (Project OGRE).
// One global block of facts for crash minidumps to carry. Evidence: iOS 1029 symbols
// RBX::RbxDbgInfo::s_instance, SetCPUName, SetServerIP, SetGfxCardName, SetGfxCardVendor,
// AddPlace(long), RemovePlace(long); DataModel calls AddPlace/RemovePlace, and
// CProcessPerfCounter::init stores NumCores. Member order and sizes are kept as the tree has
// them (NumCores' offset is part of init's code); the 2013 constructor is not placed yet.
#pragma once

#include <cstddef>

#define PLACE_HISTORY 4
#define DBG_STRING_MAX 128

namespace RBX {

struct RbxDbgInfo
{
	static RbxDbgInfo s_instance;
	RbxDbgInfo();

	// Byte counts the renderer reports: material, texture and mesh memory, and its estimate of the
	// texture memory still free.
	size_t cbMaterials, cbTextures, cbMeshes, cbEstFreeTextureMem;

	// A snapshot of the machine in the shape of PERFORMANCE_INFORMATION (psapi): commit charge and
	// limit, physical memory, page size, kernel pools, file cache, and the handle, process and
	// thread totals. The c prefix counts pages, cb counts bytes.
	size_t cCommitTotal, cCommitLimit, cPhysicalTotal, cPhysicalAvailable, cbPageSize;
	size_t cKernelPaged, cKernelNonPaged, cSystemCache, HandleCount, ProcessCount, ThreadCount;

	// Graphics adapter as the renderer saw it (SetGfxCard*), and its memory.
	char GfxCardName[DBG_STRING_MAX], GfxCardDriverVersion[DBG_STRING_MAX], GfxCardVendorName[DBG_STRING_MAX];
	size_t TotalVideoMemory;

	char CPUName[DBG_STRING_MAX]; // SetCPUName
	size_t NumCores;              // CProcessPerfCounter::init: SYSTEM_INFO::dwNumberOfProcessors

	char AudioDeviceName[DBG_STRING_MAX];
	char ServerIP[DBG_STRING_MAX]; // SetServerIP, the game server this client joined

	// The last PLACE_HISTORY places the DataModel loaded, newest in slot 0 (AddPlace shifts the
	// others down; RemovePlace takes one out), readable as an array or as four named longs.
	union {
		long PlaceIDs[PLACE_HISTORY];
		struct { long Place0, Place1, Place2, Place3; };
	};
	long PlaceCounter; // places loaded this session
	long PlayerID;

	static void SetGfxCardName(const char* s);
	static void SetGfxCardDriverVersion(const char* s);
	static void SetGfxCardVendor(const char* s);
	static void SetCPUName(const char* s);
	static void SetServerIP(const char* s);

	static void AddPlace(long ID);
	static void RemovePlace(long ID);
};

}
