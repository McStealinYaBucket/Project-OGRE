/* LegacyContentTable.h -- October 2013.
   Layout from the ctor (PLAYER 0x008f08a0, STUDIO 0x009add30): a boost::unordered_map<string, string>
   at +0x00 (prime-list bucket count from 11, max load 1.0f at +0x0c) and the empty string FindEntry
   returns on a miss at +0x18 (default-constructed at 0x008f091c). No AddEntryProd and no FFlag in
   2013 (neither the "Asset?id=%s" format nor DebugRenderDownloadAssets is in the binary).
   FindEntry is non-const (iOS _ZN3RBX18LegacyContentTable9FindEntryERKSs). */

#pragma once

#include <string>
#include <boost/unordered_map.hpp>

namespace RBX {

	// SIZE 0x34
	class LegacyContentTable
	{
	private:
		typedef boost::unordered_map<std::string, std::string> UrlMap;
		UrlMap mMap;		// 0x00
		std::string mEmpty;	// 0x18

	public:
		LegacyContentTable();

		void AddEntry(const std::string& path, const std::string& contentId);
		const std::string& FindEntry(const std::string& path);
	};

} // namespace RBX
