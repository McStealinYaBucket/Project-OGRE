/* Util/Name.h — reconstructed for October 2013.
   Evidence: symbolled Studio 0.126 decompilation (F:\2013Decompile\exports\decomp\Util_Name_core.cpp.txt)
     - ctor 0x231AA2: Name { int* sortIndex at +0; std::string str at +4 } -> setOrderIndex()
     - setOrderIndex 0x231AF4: binary-insert into a global std::vector<Name*> "ordered",
       interlocked-incrementing the sortIndex of each skipped entry
       (asserts at App/util/Name.cpp lines 98, 103)
     - lookup 0x231CF8 / 0x231DEE: guarded unordered_map<string, Name*> find; falls back to nullName
     - getNullName 0x231DBC: call_once-guarded static
     - declare 0x23206A: find_node, upgrade_to_writer, insert new Name
     - size 0x232020 / approximateMemoryUsage 0x232000: map size / size<<6
   2013 divergence from 2016: uses tbb::queuing_rw_mutex for the map guard
   (2016 uses rbx::mutex); layout has int* sortIndex pointer (not inline atomic). */

#pragma once

#include <string>
#include "rbx/atomic.h"
#include "boost/utility.hpp"
#include <boost/unordered_map.hpp>

namespace RBX {

	// SIZE 0x20
	class Name
	{
	public:
		class NameMap;			// defined in Name.cpp
		static NameMap& map();

		// 2016-compat template declare: interned static Name per string literal.
		// The 2013 binary inlines this pattern at use sites (typed_holder::construct_func
		// and the Described classDescriptor singletons), so the semantics match.
		// STUB: PLAYER 0x00480e90
		// STUB: STUDIO 0x0043f250
		template<const char* const& sName>
		static const Name& doDeclare()
		{
			static const Name* n = declare(sName);
			return *n;
		}

		// 2013 layout (ctor PLAYER 0x006136c0, setOrderIndex 0x00613530; operator new 0x20
		// in declare 0x006138c0): an inline rbx::atomic<int> (its out-of-line ctor 0x004022d0
		// zeroes it and asserts 4-byte alignment), not the Mac reading's int*.
		rbx::atomic<int> sortIndex;	// 0x00: interlocked-incremented during setOrderIndex insertion

	public:
		std::string const str;	// 0x04: the string that is the text name

		static size_t size();							// 0x232020
		static size_t approximateMemoryUsage();			// 0x232000

		// Declaration and Query
		static const Name& getNullName();				// 0x231DBC
		static const Name* lookup(const char* name);	// 0x231CF8
		static const Name* lookup(const std::string& name);	// 0x231DEE
		static const Name* declare(const char* const& name);	// 0x23206A (by reference: iOS name; getNullName's init 0x00613a70 passes &ptr)

		int getOrderIndex() const { return sortIndex; }

		// 2016-style conveniences (inlined at use sites in the 2013 binary)
		// Names are interned: == / != compare addresses (ServiceProvider::onChildAdded 0x006a9770 and
		// GuidItem::Registry::lookupByGuid 0x0055e030 cmp the getNullName() pointer; 2016 agrees).
		bool operator==(const Name& other) const { return this == &other; }
		bool operator==(const std::string& other) const { return str == other; }
		bool operator!=(const std::string& other) const { return str != other; }
		bool operator!=(const Name& other) const { return this != &other; }
		bool operator<(const Name& other) const { return str < other.str; }
		int compare(const std::string& other) const { return str.compare(other); }
		int compare(const Name& other) const { return str.compare(other.str); }
		std::string toString() const { return str; }
		const char* c_str() const { return str.c_str(); }
		bool empty() const { return str.empty(); }

		~Name();										// public: ContentId stores Name by value

	private:
		explicit Name(const char* const& name);		// 0x006136c0 (ret 4, dereferences the argument)

		void setOrderIndex();							// 0x231AF4
	};

} // namespace RBX
