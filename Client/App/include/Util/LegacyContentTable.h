/* LegacyContentTable.h — stub during bring-up.
   Evidence: global RBX::ContentId::legacyContentTable at 0xfa6740 (Mach-O bss).
   Full decompile TODO: AddEntry table of rbxasset:// sounds + FindEntry with
   lowercase-normalized unordered_map lookup (see ContentId.cpp TODO). */

#pragma once

#include <string>

namespace RBX {

	class LegacyContentTable
	{
	public:
		LegacyContentTable();
		const std::string& FindEntry(const std::string& url) const;
	};

} // namespace RBX
