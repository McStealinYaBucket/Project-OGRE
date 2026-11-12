/* V8DataModel/TouchTransmitter.h -- October 2013 RBX::TouchTransmitter.

   EVIDENCE (symbolled 0.126.0.39503 Studio, idalib):
     0x4E2FAA  TouchTransmitter::TouchTransmitter()                    451B
     0x4E31DC  TouchTransmitter::~TouchTransmitter()                   (D2)
     0x4E319E  TouchTransmitter::~TouchTransmitter()                   (D1)
     0x4E316E  TouchTransmitter::~TouchTransmitter()                   (D0)
     0x4E3296  TouchTransmitter::checkTouch(shared_ptr<PartInstance> const&)   43B
     0x4E32C2  TouchTransmitter::checkUntouch(shared_ptr<PartInstance> const&) 43B
     0x4E35CE  FactoryProduct<TouchTransmitter, Instance,
                              sTouchTransmitter>::getClassName()
     0x4E35EA  non-virtual thunk, -32   (Countable base)
     0x4E360A  ...FactoryProduct<...>::Creator::~Creator()
     0x4E3752  ...Creator::create()
     0x4E37FC  Creatable<Instance>::create<TouchTransmitter>()
     0x4E3912  Name::callDoDeclare<sTouchTransmitter>()
     0x4E3984  ...Creator::Creator()
     0x4E3BE2  ...Creator::static_getCreator()
     0x4E3D0A  DescribedCreatable<TouchTransmitter, Instance,
               sTouchTransmitter, Functionality::eNone, Permissions::eNone>::~()
     0x4E3DE8  Reflection::Described<TouchTransmitter, ...>::classDescriptor()
     0x4E3EE4  non-virtual thunks -32 / -36
     0x4F25A3  boost::shared_ptr<TouchTransmitter>(...,
                        Creatable<Instance>::Deleter)
     0x4E3C66  boost::scoped_ptr<TouchDebouncer>::~scoped_ptr()
     0x4E3CA0  std::vector<TouchDebouncer::Item>::~vector()
   Plus, in the same address range, the rbx::signals machinery for
   `signal<void(const TouchPair&)>` -- connect 0x87A438 (binding
   Network::PhysicsSender::onTouchStep 0x878F84), insert 0x87AFDE,
   next 0x51A6A6, safe_static_init_mutex 0x51A788, disconnectAll 0x522562 --
   and a `std::vector<TouchPair>` (resize 0x518B18, _M_fill_insert 0x51A80E,
   std::fill 0x51ACAD).

   CLASS SIZE: the two non-virtual thunks are at -32 and -36, so the two
   polymorphic bases sit at this+0x20 and this+0x24 and the vtable is at
   this+0 -- the standard Countable/DescribedCreatable/FactoryProduct shape,
   which `DescribedCreatable<TouchTransmitter, Instance, sTouchTransmitter>`
   already produces.

   DEFAULT NAME: 0x4E2FAA ends with
       std::string s("TouchInterest"); Instance::setName(this, s);
   so a freshly created TouchTransmitter is named "TouchInterest", NOT after
   its class.

   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/TouchTransmitter.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8DataModel/TouchDebouncer.h"
#include "V8DataModel/TouchPair.h"

namespace RBX {

	class PartInstance;

	extern const char* const sTouchTransmitter;

	class TouchTransmitter
		: public DescribedCreatable<TouchTransmitter, Instance, sTouchTransmitter>
	{
	public:
		// mac: 0x4E2FAA (451B): Instance::Instance(NULL), the vtable fixups for
		// each of the four bases, the reflection class descriptor, the
		// Diagnostics::Countable bump, `this+0x5C = new TouchDebouncer`
		// (operator new(0xC)) and setName("TouchInterest").
		TouchTransmitter();
		virtual ~TouchTransmitter();

		// mac: 0x4E3296 (43B) and 0x4E32C2 (43B).  Both are the same three
		// instructions -- load this+0x5C, tail-call TouchDebouncer::check
		// with the pair and a literal 0 (resp. 1) -- so:
		//     return debouncer->check(part, TouchPair::Touched);
		//     return debouncer->check(part, TouchPair::Untouched);
		// They are the entire touch-debouncing front end; PartInstance's
		// reportTouch/reportUntouch call these and only fire the Touched /
		// Untouch signals when the answer is true.
		bool checkTouch(const shared_ptr<PartInstance>& part);
		bool checkUntouch(const shared_ptr<PartInstance>& part);

	private:
		// +92 (0x5C) -- the ONLY member TouchTransmitter adds.  A
		// boost::scoped_ptr in the binary (its out-of-line destructor is
		// emitted at 0x4E3C66 right next to the vector<Item> destructor);
		// a raw owning pointer here, new'd and delete'd inside
		// v8datamodel/TouchTransmitter.cpp so the mixed-CRT rule holds
		// either way.
		//
		// DELTA: the two `signal<void(const TouchPair&)>` members and the
		// `std::vector<TouchPair>` that the binary carries (their whole
		// rbx::signals instantiation set is listed at the top of this file)
		// are NOT ported.  They are the accumulated live-touch set that
		// reportTouch/reportUntouch maintain, they need PartInstance's
		// signal slots (OnDemand +0x48/+0x50/+0x60/+0x70) to be wired up,
		// and neither reportTouch (0x44A8F0) nor reportUntouch (0x445590)
		// is ported yet.  Declaring them here without bodies would emit a
		// vtable that the binary does not have, so they stay undeclared and
		// the gap is recorded in LAYOUT_AUDIT.md.
		TouchDebouncer* debouncer;
	};

} // namespace RBX
