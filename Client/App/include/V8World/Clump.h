/* V8World/Clump.h -- October 2013 (primitive-group node).
   Evidence: Clump C1 0x55D1BA (batch269: IndexedMesh C2 + own vtable
   0xEEE088 -- single inheritance, no added members) /
   getPrimitiveClump 0x55D23E (batch258/269: single-hop
   getComputedUpper from prim+8, NO -8 adjust -- the upper IS the Clump
   object, valid since Clump : IndexedMesh sits at +0).
   The group-build sites (who links prim meshes under a Clump) queue
   with the Clump-construction flow; the lookup + link primitives are
   live here.
   Original file: Client/App/include/V8World/Clump.h (path inferred). */

#pragma once

#include "Util/IndexedMesh.h"

namespace RBX {

	class Primitive;

	class Clump
		: public IndexedMesh
	{
	public:
		// mac: 0x55D1BA (batch269: base C2 + vtable -- compiler does both).
		Clump()
			: IndexedMesh()
		{}

		// mac: 0x55D23E (batch258/269: static in spirit -- no this-use;
		// member per the mangling).
		static Clump* getPrimitiveClump(Primitive* prim);
		static const Clump* getConstPrimitiveClump(const Primitive* prim);
	};

} // namespace RBX
