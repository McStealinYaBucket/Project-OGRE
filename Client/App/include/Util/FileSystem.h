/* Util/FileSystem.h — October 2013 (minimal bring-up).
   Evidence: RBX::FileSystem::getUserDirectory(std::string&) callee of
     GetCustomStatsFilename 0x395FB6 (GuiJSON_file.txt).
   TODO(decompile): real user-directory resolution (2013 location scheme) +
     the rest of the FileSystem surface.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/include/util/FileSystem.h */

#pragma once

#include <string>
#include <boost/filesystem/path.hpp>

namespace RBX {
	// 2013 values: getUserDirectory 0x006ea4c0 tests dir == 3 for the exe directory, and its two
	// folder switches take 0 (LocalAppData), 1 (Pictures), 2 (Videos).
	enum FileSystemDir
	{
		DirAppData = 0,
		DirPicture,
		DirVideo,
		DirExe
	};

	namespace FileSystem {

		// PLAYER 0x006ea4c0 (util/FileSystem.cpp): <folder>\Roblox\[subDirectory]\, created on
		// request; "" when it cannot be made.
		std::string getUserDirectory(bool create, FileSystemDir dir, const char* subDirectory);

		// mac: 0x395FB6 callee. TODO: real path (usually %APPDATA%/... or
		// Documents/... in 2013 — verify from the FileSystem module).
		void getUserDirectory(std::string& result);

		// mac: 0x1F9A5E (858B, batches 676/678): mktemp-unique base + "/Roblox",
		// optional create (throws "Unable to create %s"), exists-probe.
		std::string getBaseCacheDirectory(bool create);
		// mac: 0x1F9848 (533B, batch677): NULL-sub delegates to base; else base
		// +/= sub, optional create (throws), exists-probe, trailing slash.
		std::string getCacheDirectory(bool create, const char* sub);
		// boost::filesystem::exists wrapper (used by the cache probers).
		bool filepathExists(const boost::filesystem::path& p);

		// mac: 0x1F969E (352B, between getCacheDirectory and getBaseCacheDirectory
		// in the image).  The only 2013 call site is
		// ContentProvider::~ContentProvider 0x60960A with ("ContentProvider", 0);
		// the mode int is not decode-evidenced (the 2016 carry dropped it).
		// Body carried from 2016 FileSystemIndependent.cpp: removes the
		// regular files (non-recursive) under the subsystem cache directory.
		void clearCacheDirectory(const char* subDirectory, int mode);

	} // namespace FileSystem
} // namespace RBX
