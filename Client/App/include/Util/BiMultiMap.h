/* Util/BiMultiMap.h -- October 2013 reconstruction. Unique (Left, Right) pairs in one
   std::multimap keyed on Left (JointStage, EdgeBuffer). The removePair asserts sit on
   PLAYER/STUDIO's recorded lines 40 and 44 ("!pairInMap(left, right)", "0"). */
#pragma once

#include "rbx/Debug.h"
#include <map>

namespace RBX {

template <typename Left, typename Right>
class BiMultiMap
{
public:
	typedef std::multimap<Left, Right> InternalMap;
	typedef typename InternalMap::iterator InternalMapIt;
	InternalMap internalMap;                              // JointStage inserts directly

	bool pairInMap(const Left& left, const Right& right)  // scans left's range
	{
		InternalMapIt pos;
		for (pos = internalMap.lower_bound(left); pos != internalMap.upper_bound(left); ++pos)
			if (pos->second == right)
				return true;
		return false;
	}
	void insertPair(const Left& left, const Right& right)
	{
		RBXASSERT_SLOW(!pairInMap(left, right));
		internalMap.insert(std::make_pair(left, right));
	}
	void removePair(const Left& left, const Right& right) // exactly one must exist
	{
		RBXASSERT_SLOW(pairInMap(left, right));
		InternalMapIt pos;
		for (pos = internalMap.lower_bound(left); pos != internalMap.upper_bound(left); ++pos)
			if (pos->second == right)
			{
				internalMap.erase(pos);
				RBXASSERT(!pairInMap(left, right));
				return;
			}
		// fell off the range: the pair was never inserted
		RBXASSERT(0);
	}

	bool empty() const
	{
		return internalMap.empty();
	}
	bool emptyLeft(const Left& left) const                // nothing paired with left
	{
		return internalMap.lower_bound(left) == internalMap.upper_bound(left);
	}

	template <class Func>                                 // func(left, r) per partner r
	inline void visitEachLeft(const Left& left, const Func& func) const
	{
		typename InternalMap::const_iterator pos;
		for (pos = internalMap.lower_bound(left); pos != internalMap.upper_bound(left); ++pos)
			func(left, pos->second);
	}
};

} // namespace RBX

/* 2013 instances. PLAYER emits BiMultiMap<Primitive*, Joint*>::pairInMap at
   0x009977b0 and removePair at 0x00997ab0 (JointStage's jointMap, Mac name
   _ZN3RBX10BiMultiMapIPNS_9PrimitiveEPNS_5JointEE10removePairERKS2_RKS4_);
   JointStage keeps the map at +0x34 and its destructor tests empty().
   EdgeBuffer holds a BiMultiMap<Assembly*, Edge*>. Each joint is filed under
   both of its primitives, so a joint appears twice, once per key. */
