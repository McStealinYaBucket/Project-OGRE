/* V8DataModel/TouchPair.h -- October 2013 RBX::TouchPair.

   EVIDENCE (symbolled 0.126.0.39503 Studio, idalib):
     0x87857A  RBX::TouchPair::TouchPair(boost::shared_ptr<PartInstance> const&,
                                        boost::shared_ptr<PartInstance> const&,
                                        TouchPair::Type)          68B
     0x51AD7E  RBX::TouchPair::operator=(TouchPair const&)          59B
     0x517A23  RBX::hash_value(RBX::TouchPair const&)
     0x518B18  std::vector<TouchPair>::resize(size_t, TouchPair)
     0x518BCA  rbx::signals::signal_with_args<1, void(TouchPair const&)>
     0x51A6A6  rbx::signals::signal<void(TouchPair const&)>::next(...)
     0x51A792  rbx::signals::signal<void(TouchPair const&)>::safe_static_do_get_mutex()
     0x51A7xx  rbx::signals::signal<void(TouchPair const&)>::connect(...)
               ... feeding RBX::Network::PhysicsSender::onTouchStep 0x878F84
     0x522562  rbx::signals::signal<void(TouchPair const&)>::disconnectAll()
     0x522A23  TouchTransmitter (see TouchTransmitter.h) owns both signals.

   LAYOUT -- exactly 20 bytes, pinned by TouchPair::TouchPair 0x87857A:
     +0   shared_ptr<PartInstance>   (dwords 0,1 -- raw + count)
     +8   shared_ptr<PartInstance>   (dwords 2,3)
     +16  Type                        (dword 4, `mov [eax+10h], edx`)
   The ctor is the two copies verbatim: each argument pair is stored and the
   count pointer is then `lock inc`d, which is just boost::shared_ptr's copy
   constructor.  So the class is a plain three-member aggregate with a
   three-argument ctor and the compiler-generated copy/assign (operator= at
   0x51AD7E delegates to shared_ptr::operator= for the two pointers and then
   copies the Type -- the same thing the implicit one does).

   DELTA: the `part0`/`part1` member NAMES are not recoverable.  The class is
   only ever built at a touch site where the roles are "the part that owns the
   transmitter" and "the part that touched it"; the two names below follow
   that reading and the 2016 source.  `hash_value` (0x517A23) shows the pair
   is used as a hash key, so the pair is order-significant.

   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/TouchPair.h */

#pragma once

// Util/Object.h is the header that does `using boost::shared_ptr;` at global
// scope (line 20), which is how every unqualified `shared_ptr` in the RBX
// headers resolves.  V8Tree/Object.h does not exist.
#include "Util/Object.h"

namespace RBX {

	class PartInstance;

	class TouchPair
	{
	public:
		enum Type
		{
			// The two VALUES are pinned by the only two call sites, which
			// pass the type as an immediate:
			//   TouchTransmitter::checkTouch   0x4E3296  `mov [esp+8], 0`
			//   TouchTransmitter::checkUntouch 0x4E32C2  `mov [esp+8], 1`
			// The ENUMERATOR NAMES are not recoverable (an enumerator is not
			// a symbol).  checkTouch/checkUntouch are the only two callers
			// in the whole binary, so Touched=0 / Untouched=1 is the only
			// reading the evidence supports.
			Touched = 0,
			Untouched = 1
		};

		TouchPair(
			const shared_ptr<PartInstance>& part0,
			const shared_ptr<PartInstance>& part1,
			Type type)
			: part0(part0)
			, part1(part1)
			, type(type)
		{
		}

		shared_ptr<PartInstance> part0;
		shared_ptr<PartInstance> part1;
		Type type;
	};

} // namespace RBX
