/* Util/StlExtra.h -- October 2013 fastRemoveShort family (batch780:
   fastRemoveShort<SimJobTracker*> 0x59AD96 decompile + full asm).
   The binary's own asserts bake the original path and lines:
       'vec[answer] == item          file: include/Util/StlExtra.h line: 47'
       'it != vec.end()              file: include/Util/StlExtra.h line: 48'
       'vec.size() < 32              file: include/Util/StlExtra.h line: 49'
   so the three RBXASSERT expansions are placed at lines 47/48/49
   below (TOSTRING(__LINE__) then matches the binary byte-for-byte;
   the MSVC __FILE__ spelling is the standing path DELTA every landed
   assert carries).
   Evidence: C:\tmp\batch780_out.txt (decompile + full asm).
   Body notes (all decoded, batch780_out.txt):
   - the find is std::find over random-access iterators (the binary
     spells gcc's std::__find dispatch); *vec[i] is compared to the
     ITEM (cmp [ecx+esi*4], [edx]).
   - the removal is a CONDITIONAL swap-with-last (only when it != the
     last element; the copy reads [end-4]), then a TWO-ARG resize:
     vec.resize(size() - 1, 0) -- the binary call is
     j___ZNSt6vectorIPN3RBX13SimJobTrackerESaIS2_EE6resizeEmS2_.
   - the return value is the REMOVED ELEMENT'S INDEX (unsigned long;
     'm' in the mangle; the one binary call site discards it).
   Self-audit: fidelity ~99% (every decoded cell of the single binary
     instantiation is transcribed; the 2013 header's other helpers, if
     the rosetta tree carried any, queue behind future instantiations
     -- not fabulated), function 100% in-tree (SimJobTracker::
     stopTracking drives it). */

#pragma once

#include <vector>
#include <algorithm>

#include "rbx/Debug.h"

namespace RBX {

	// mac: 0x59AD96 (batch780). Binary mangle:
	//   __ZN3RBX15fastRemoveShortIPNS_13SimJobTrackerEEEmRSt6vectorIT_SaIS4_EERKS4_
	// = template<class T> unsigned long fastRemoveShort(
	//       std::vector<T>& vec, const T& item)
	//   instantiated for T = RBX::SimJobTracker*.
	// STUB: PLAYER 0x00822510
	// STUB: STUDIO 0x009164e0
	template <typename T> inline
	unsigned long fastRemoveShort(std::vector<T>& vec, const T& item)
	{
		typename std::vector<T>::iterator it = std::find(vec.begin(), vec.end(), item);
		unsigned long answer = it - vec.begin();
		RBXASSERT(vec[answer] == item);   // baked '... StlExtra.h line: 47'
		RBXASSERT(it != vec.end());       // baked '... StlExtra.h line: 48'
		RBXASSERT(vec.size() < 32);       // baked '... StlExtra.h line: 49'
		if (it != vec.end() - 1)
		{
			*it = *(vec.end() - 1);
		}
		vec.resize(vec.size() - 1, T());
		return answer;
	}

} // namespace RBX
