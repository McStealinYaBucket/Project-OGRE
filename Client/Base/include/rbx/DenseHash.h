// Base/include/rbx/DenseHash.h: October 2013 reconstruction (Project OGRE).
// The 2016 file of this name holds a shared open-addressing table (DenseHashTable) with
// DenseHashSet and DenseHashMap on top. October 2013 has none of that:
//   - no DenseHashMap or DenseHashTable symbol on Mac or iOS, and no such RTTI or string in PLAYER;
//   - 2013's open-addressing set is the standalone rbx/DenseHashSet.h (its own asserts at lines 19,
//     24, 57 and 189 in PLAYER; 14 RBX::DenseHashSet<...> symbols on iOS 1029), which this tree
//     already reconstructs;
//   - the two 2016 users are reflection's member and class containers, and the 2013 constructors
//     show other layouts there (reflection/member.h: 2016's 0x1c DenseHashMap is absent;
//     reflection/object.h: the class is 0x16c only in 2016).
// So nothing of the 2016 body is kept. reflection/member.h still includes this file and may rely
// on what it pulls in, so the includes stay. Do not add a second RBX::DenseHashSet here: it would
// clash with rbx/DenseHashSet.h.
#pragma once

#include <boost/functional/hash.hpp>
#include <vector>

#include "rbx/Debug.h"
