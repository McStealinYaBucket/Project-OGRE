/* V8DataModel/TouchDebouncer.h -- October 2013 RBX::TouchDebouncer.

   EVIDENCE (symbolled 0.126.0.39503 Studio, idalib):
     0x4E32F6  RBX::TouchDebouncer::check(shared_ptr<PartInstance> const&,
                                          TouchPair::Type)        623B
     0x4E3C66  boost::scoped_ptr<RBX::TouchDebouncer>::~scoped_ptr()
     0x4E3CA0  std::vector<RBX::TouchDebouncer::Item>::~vector()
     0x4E3F90  std::vector<Item>::resize(size_t, Item)
     0x4E3FEE  std::vector<Item>::push_back(Item const&)
     0x4E4056  std::vector<Item>::_M_insert_aux(...)
     0x4E431E  std::_Vector_base<Item>::_M_allocate(size_t)
     0x4E4344  std::__copy_backward<Item*, Item*>::__copy_b(...)
     0x4E43C4  std::vector<Item>::_M_erase_at_end(Item*)
     0x4E4410  std::vector<Item>::_M_fill_insert(...)
   TouchTransmitter::TouchTransmitter 0x4E2FAA does `operator new(0xC)` for its
   debouncer, so the class is EXACTLY 12 bytes -- a bare std::vector<Item>
   (begin/end/end-of-capacity), with no vtable and no other members.

   `check` is the class's ONLY member function: a full symbol sweep for
   TouchDebouncer turns up that one method plus the out-of-line std::vector
   instantiations above.  There is no ctor, no dtor, no reset, no empty.

   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/TouchDebouncer.h */

#pragma once

#include <vector>

// rbx/rbxtime.h IS the authentic 2013 RBX::Time class (see Client/Base/).
// `private: double sec;` and a PROTECTED `Time(double)` ctor, which is what
// makes `static Time expiration = 5.0` unspellable and forces the Interval
// spelling that the binary's single `addsd` implies.
#include "rbx/rbxtime.h"
#include "V8DataModel/TouchPair.h"

namespace RBX {

	class PartInstance;

	class TouchDebouncer
	{
	public:
		// EXACTLY 20 bytes.  The stride is explicit in the disasm
		// (`imul edi, 20` / `20 * v5` at every access) and independently
		// confirmed by the size-division magic 0xCCCCCCCD in check, which
		// is the compiler's divide-by-5 on a (byte_count >> 2) count --
		// i.e. 5 dwords == 20 bytes.  Field offsets:
		//   +0   shared_ptr<PartInstance> part
		//        compared BY RAW POINTER at 0x4E334C (`mov v7, [base+20*i]`
		//        against `mov v26, [part]`), never by shared_count
		//   +8   TouchPair::Type type      (`cmp dword [base+20*i+8], type`)
		//   +12  Time expiration          (`movsd [base+20*i+12], xmm0`)
		//
		// The +12 offset for an 8-byte double is what fixes the alignment
		// of the whole struct at FOUR, not eight: the 2013 build is i386
		// System V, where __alignof__(double) == 4, so 8+4 leaves no
		// padding and the stride is exactly 20 with every expiration
		// unaligned.  MSVC x86 defaults double to 8-byte alignment, which
		// would pad to 24, so the packing is requested explicitly.  It is
		// safe to do so here: Item only ever lives inside this class's
		// vector, which never leaves App/v8datamodel.
#pragma pack(push, 4)
		struct Item
		{
			shared_ptr<PartInstance> part;
			TouchPair::Type type;
			Time expiration;
		};
#pragma pack(pop)

		// mac: 0x4E32F6.  Returns true when the pair state actually changed, which
		// is what makes it a DEBOUNCER: a repeated Touched inside the
		// expiration window returns false and the caller does not re-fire
		// the signal.  See v8datamodel/TouchDebouncer.cpp for the body.
		bool check(const shared_ptr<PartInstance>& part, TouchPair::Type type);

	private:
		std::vector<Item> items;
	};

} // namespace RBX
