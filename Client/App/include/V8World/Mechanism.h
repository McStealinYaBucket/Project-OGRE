/* V8World/Mechanism.h -- October 2013 (mechanism shell).
   Evidence: Mechanism C2 0x57DDB4 (96B): IPipelined base (vptr + +4 = 0),
     IndexedMesh member at +8; getPrimitiveMechanism 0x57E0BA (50B) +
     getConstPrimitiveMechanism 0x57E0EC (50B, symmetric): Assembly lookup
     then one computedUpper hop with -8 owner adjust, NULL-safe.
   DELTAS: +4 int identity TODO; the secondary +8 vtable (off_EEE840)
     unmodeled MI-flattening; isComplexMovingMechanism/isMovingAssemblyRoot/
     getMovingAssemblyRoot/getRootMovingPrimitive/getRootAssembly/
     isMechanismRootPrimitive + D0/D1/D2 queue behind Assembly linkage.
   LAYOUT FIX (section 185): the old `int unknown04` member double-mapped
     IPipelined's +4 (currentStage, already zeroed by the base C2) and
     pushed `mesh` to +12; the -8 walks need it at +8. Removed (no other
     readers); mesh now sits at +8 binary-exact.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/Mechanism.h (path inferred) */

#pragma once

#include "V8World/IPipelined.h"
#include "Util/IndexedMesh.h"

namespace RBX {

	class Primitive;
	class Assembly;

	class Mechanism
		: public IPipelined
	{
		IndexedMesh mesh;  // +8 (binary-exact: the -8 walks assume this).
	public:
		Mechanism();

		// mac: 0x57E0BA (50B) / 0x57E0EC (50B, symmetric): Assembly lookup + one
		// computedUpper hop with -8 adjust.  Static in spirit (no this-use;
		// member per the mangling).
		static Mechanism* getPrimitiveMechanism(Primitive* prim);
		static const Mechanism* getConstPrimitiveMechanism(const Primitive* prim);

		// mac: 0x57DEBE (28B, const): the owner primitive of this mechanism's
		// upper chain: deref this+0x28 -> +0x20 -> +0x20, null-test only the
		// final pointer, strip -8 (the IPipelined owner slot, same -8 shape
		// as the computedUpper walks).  DELTA: the two front hops ride
		// unpoured chain storage (their slots are untyped in this shell);
		// the cpp body guards them where the binary does not.
		Primitive const* getConstMechanismPrimitive() const;

		// Batch269 (2016 static shapes): ungrounded + no-moving-parent
		// test (0x57DF74) and moving-root climb (0x57DFAC). The binary
		// spells the parent hop as mesh-word -8 (mesh-at-+8 owner
		// convention, same as the getPrimitiveAssembly walks); spelled
		// here through the mesh-parent accessor with that note.
		static bool isMovingAssemblyRoot(const Assembly* assembly);
		static Assembly* getMovingAssemblyRoot(Assembly* assembly);
		static const Assembly* getConstMovingAssemblyRoot(const Assembly* assembly);
		// mac: 0x57E11E (batch281: lower-mesh -8 container + :127 answer
		// assert. Asserts on fresh mechanisms (no lower) -- link-covered;
		// runs with staged mechanisms).
		Assembly* getRootAssembly();

		IndexedMesh* getMesh() { return &mesh; }
		const IndexedMesh* getMesh() const { return &mesh; }
		// 2016 shape (cleanMechanism asserts the mechanism mesh is
		// unparented).
		IndexedMesh* getIndexedMeshParent() { return getMesh()->getIndexedMeshParent(); }
	};

} // namespace RBX
