/* Name.cpp — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation (F:\2013Decompile\exports\decomp\Util_Name_core.cpp.txt)
     Original path: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/util/Name.cpp
     (assert lines 98, 103)
   2013 divergences from 2016 implemented here:
     - NameMap guard is tbb::queuing_rw_mutex (acquire/read, upgrade_to_writer for insert)
     - map is boost::unordered_map<std::string, Name*>
     - "ordered" global std::vector<Name*> maintained in sort order with interlocked
       increment of each skipped entry's sortIndex */

#include "Util/Name.h"
#include "decomp.h"
#include "rbx/Debug.h"	// OGRE vc9: RBXASSERT and <windows.h> (InterlockedIncrement) were not reachable
#include <tbb/queuing_rw_mutex.h>
#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <algorithm>

// getNullName's call_once state (file statics outside RBX: the PDB names them unqualified).
// GLOBAL: PLAYER 0x00de2810
// GLOBAL: STUDIO 0x00f69544
static const RBX::Name* nullName;
// GLOBAL: PLAYER 0x00de286c
// GLOBAL: STUDIO 0x00f695a0
static boost::once_flag nullNameFlag = BOOST_ONCE_INIT;

namespace RBX {

	//////////////////////////////////////////////////////////////////////////////////
	// NameMap
	//////////////////////////////////////////////////////////////////////////////////
	class Name::NameMap
	{
	public:
		typedef boost::unordered_map<std::string, Name*> StringToName;

		tbb::queuing_rw_mutex mutex;
		boost::unordered_map<std::string, Name*> hash;	// at unk_FAE354 in the binary
		std::vector<Name*> ordered;						// at dword_FAE380, sorted by str

		size_t size() const { return hash.size(); }
	};

	static Name::NameMap* s_map = NULL;	// OGRE vc9: nullptr is C++11
	static Name* s_nullName = NULL;	// OGRE vc9: nullptr is C++11

	Name::NameMap& Name::map()
	{
		static NameMap theMap;	// call_once-guarded in the original (unk_FAE350)
		return theMap;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////////////
	// FUNCTION: PLAYER 0x006136c0
	// FUNCTION: STUDIO 0x00702f90
	Name::Name(const char* const& name)
		: str(name)
	{
		setOrderIndex();
	}

	Name::~Name()
	{
	}

	// STUB: PLAYER 0x00613530
	// STUB: STUDIO 0x00702e00
	void Name::setOrderIndex()	// 0x231AF4
	{
		NameMap& m = map();
		int index = 0;

		// walk backwards through the ordered list; each skipped name's sortIndex
		// is incremented (interlocked), asserting invariants along the way
		size_t i = m.ordered.size();
		while (i > 0) {
			Name* prev = m.ordered[i - 1];
			RBXASSERT(prev->str != str);		// "ordered[i-1]->str != str" (Name.cpp:98)

			if (str.compare(prev->str) > 0) {
				index = static_cast<int>(i);
				break;
			}

			++prev->sortIndex;
			RBXASSERT(prev->getOrderIndex() == static_cast<int>(i));	// "ordered[i-1]->sortIndex == i" (Name.cpp:103)

			--i;
		}

		sortIndex = index;
		m.ordered.insert(m.ordered.begin() + index, this);
	}

	const Name* Name::lookup(const char* name)	// 0x231CF8
	{
		if (name)
			return lookup(std::string(name));
		return &getNullName();
	}

	const Name* Name::lookup(const std::string& name)	// 0x231DEE
	{
		NameMap& m = map();
		tbb::queuing_rw_mutex::scoped_lock lock(m.mutex, false);	// reader

		NameMap::StringToName::const_iterator it = m.hash.find(name);
		if (it != m.hash.end())
			return it->second;
		return &getNullName();
	}

	// STUB: PLAYER 0x006138c0
	// STUB: STUDIO 0x00703190
	const Name* Name::declare(const char* const& name)	// 0x23206A
	{
		if (!name)
			return &getNullName();

		NameMap& m = map();
		tbb::queuing_rw_mutex::scoped_lock lock(m.mutex);	// reader first

		NameMap::StringToName::const_iterator it = m.hash.find(name);
		if (it != m.hash.end())
			return it->second;

		lock.upgrade_to_writer();
		Name* n = new Name(name);
		m.hash[name] = n;
		return n;
	}

	// call_once-guarded static (mac 0x231DBC). 0x0090e3d0 / STUDIO 0x00547620, which the prior markers named,
	// are a constructor that stores getNullName() at +0x1c, not this function.

	// FUNCTION: PLAYER 0x00613a70
	// FUNCTION: STUDIO 0x00703340
	static void initNullName()
	{
		nullName = Name::declare("");
	}

	// FUNCTION: PLAYER 0x00613a90
	// FUNCTION: STUDIO 0x00703360
	const Name& Name::getNullName()
	{
		boost::call_once(&initNullName, nullNameFlag);
		return *nullName;
	}

	size_t Name::size()	// 0x232020
	{
		return map().size();
	}

	size_t Name::approximateMemoryUsage()	// 0x232000: size << 6
	{
		return map().size() << 6;
	}

} // namespace RBX

DECOMP_SIZE_ASSERT(RBX::Name, 0x20);
