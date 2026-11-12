/* Copyright 2003-2005 ROBLOX Corporation, All Rights Reserved */
//
// Reconstructed from Oct 2013 binary. Assert strings embedded in the binary
// pin the original source line numbers exactly onto this file:
//   util/IndexedTree.cpp line 17: RBXASSERT(children.size() == 0)
//   util/IndexedTree.cpp line 18: RBXASSERT(!parent)
//   util/IndexedTree.cpp line 19: RBXASSERT(index == -1)
//   util/IndexedTree.cpp line 42: RBXASSERT(!newParent || (newParent->parent != this))
//   util/IndexedTree.cpp line 43: RBXASSERT(newParent != this)
// Matches the 2016 reference implementation.
//
#include "stdafx.h"

#include "Util/IndexedTree.h"


namespace RBX {


// mac: 0x2068AA
IndexedTree::IndexedTree()
: parent(NULL)
, index(-1)
{
}

// mac: 0x20691A (complete object dtor); deleting dtor 0x206910 wraps this + operator delete
// STUB: PLAYER 0x00999690
// STUB: STUDIO 0x00a43f50
IndexedTree::~IndexedTree()
{
	RBXASSERT(children.size() == 0);
	RBXASSERT(!parent);
	RBXASSERT(index == -1);
}



// Make sure newAncestor and all parents do not have child as their parent
bool IndexedTree::circularReference(IndexedTree* newAncestor, IndexedTree* child)
{
	RBXASSERT(child);
	if (!newAncestor) {
		return false;
	}
	else if (newAncestor->parent == child) {
		return true;
	}
	else {
		return circularReference(newAncestor->parent, child);
	}
}


// mac: 0x206A62
// STUB: PLAYER 0x00999850
// STUB: STUDIO 0x00a44060
void IndexedTree::setIndexedTreeParent(IndexedTree* newParent)
{
	RBXASSERT(!newParent || (newParent->parent != this));
	RBXASSERT(newParent != this);
	RBXASSERT_VERY_FAST(!circularReference(newParent, this));

	if (parent != newParent)
	{
		IndexedTree* oldParent = parent;

		onParentChanging();

		if (parent) {
			parent->onChildRemoving(this);
			parent->children.fastRemove(this);
			parent->onChildRemoved(this);				// parent Cofm will be set dirty here - mine shouldn't change!
		}

		parent = newParent;

		if (parent) {
			parent->onChildAdding(this);
			parent->children.fastAppend(this);
			parent->onChildAdded(this);
		}

		onParentChanged(oldParent);
	}
}


} // namespace

// ============================================================================
// TODO / ownership notes (out-of-module functions present in the export
// Util2_IndexedTree.cpp.txt, not written here):
//
// - G3D::Array<RBX::IndexedTree*, 10, 32> ctor/dtor/append/resize/realloc
//   (0x207098, 0x207034, 0x206D8E, 0x206E04, 0x206ECE): provided by the
//   vendored G3D under Rendering\g3d\ (G3D/Array.h, included via
//   Util/IndexArray.h). Header param list matches the vendored template
//   (MIN_ELEMENTS = 10, MIN_BYTES = 32). Do not re-implement.
//
// - RBX::IndexArray<IndexedTree, &IndexedTree::getIndex>::fastRemove
//   (0x206C86): lives in Util/IndexArray.h (template).
//
// - Template instantiations of IndexedTree members (thunks + bodies):
//   getTypedChild<IndexedMesh> 0x206706/0x95AEDA,
//   getTypedChild<Primitive> 0x555DC4/0x96668E,
//   getOneBelowRoot<Assembly> 0x59BCA6/0x9673F0,
//   visitMeAndChildren<Assembly, boost::bind...> 0x57E8B2/0x5A653A + thunks,
//   visitConstMeAndChildren<Assembly, boost::bind...> 0x87A850/0x93D81A + thunks.
//   All are header templates instantiated by other modules; their bodies match
//   the header templates above (note the itanium->MSVC quirk: the binary
//   implements rbx_static_cast as a -8 offset from the IndexArray container
//   back to the object base).
//
// - RBX::IndexedMesh::onParentChanged 0x2064E4: belongs to Util/IndexedMesh.
// - RBX::Body::onParentChanged 0x52459E, onChildAdding 0x524628,
//   onChildAdded 0x5247E8, onChildRemoved 0x5248BA: belong to v8kernel/Body.
// ============================================================================
