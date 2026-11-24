/* Util/ComputeProp.h -- October 2013 (lazily computed member value).
   2013 evidence: Primitive's ctor 0x00756a40 builds one at +0x158 for its joint stiffness:
   {Type val +0x00 (left unset), bool dirty +0x04 (true), O* object +0x08 (this),
   Type (O::*getFunc)() +0x10}. The member-function pointer is two dwords under the MI
   model and 8-aligned, so +0x0c is padding and sizeof is 0x18 for <float, Primitive>.
   The ctor takes getFunc as an argument, so &computeJointK is loaded before the
   dirty/object stores. Only the ctor is proven; getValue/setDirty are the obvious users. */

#pragma once

namespace RBX {

	template<class Type, class O>
	class ComputeProp
	{
	public:
		typedef Type (O::*GetFunc)();

		ComputeProp(O* object, GetFunc getFunc)
			: dirty(true)
			, object(object)
			, getFunc(getFunc)
		{}

		Type getValue()
		{
			if (dirty) {
				val = (object->*getFunc)();
				dirty = false;
			}
			return val;
		}

		void setDirty() { dirty = true; }

	private:
		Type val;
		bool dirty;
		O* object;
		GetFunc getFunc;
	};

} // namespace RBX
