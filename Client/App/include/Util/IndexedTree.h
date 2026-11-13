/* Copyright 2003-2006 ROBLOX Corporation, All Rights Reserved */
// Reconstructed from Oct 2013 binary (Mach-O i386 studio):
//   ctor 0x2068AA, dtor 0x206910/0x20691A, setIndexedTreeParent 0x206A62, empty virtual stubs 0x2067B8 (onChildRemoving), 0x206D88 (onParentChanged), mac: 0x555D40 (onChildAdded), 0x55D6C8 (onChildAdding), 0x55D6D0 (onAncestorChanged), mac: 0x57E268 (onParentChanging), 0x57E270 (onChildRemoved). Matches the 2016 reference header; the binary's assert strings ("include/Util/IndexedTree.h" line 78 for getOneBelowRoot) confirm layout: +0 vtable, +4 parent, +8 index, +12 children (IndexArray -> G3D::Array<IndexedTree*, 10, 32>).
#pragma once
#include "rbx/Declarations.h"
#include "Util/IndexArray.h"
#include "rbx/Debug.h"
namespace RBX {
	class SpanningEdge;
	class RBXBaseClass IndexedTree
	{
		// Binary calls setIndexedTreeParent from SpanningEdge helpers (addTo/removeFromSpanningTree -- no access control at runtime); friendship spells the same reachability here.
		friend class SpanningEdge;
	private:
		IndexedTree*				parent;
		int							index;
		int&						getIndex() {return index;}
		IndexArray<IndexedTree, &IndexedTree::getIndex> children;
		bool circularReference(IndexedTree* newAncestor, IndexedTree* child);
	protected:
		// STUB: PLAYER 0x00528720
		virtual void onParentChanging() {}						// 0x57E268
		virtual void onParentChanged(IndexedTree* oldParent) {}	// 0x206D88
		// FUNCTION: PLAYER 0x00575d00 FOLDED
		// FUNCTION: STUDIO 0x007aa330 FOLDED
		virtual void onChildAdding(IndexedTree* child) {}		// 0x55D6C8
		// FUNCTION: PLAYER 0x00575d00 FOLDED
		// FUNCTION: STUDIO 0x007aa330 FOLDED
		virtual void onChildAdded(IndexedTree* child) {}		// 0x555D40
		// FUNCTION: PLAYER 0x00575d00 FOLDED
		// FUNCTION: STUDIO 0x007aa330 FOLDED
		virtual void onChildRemoving(IndexedTree* child) {}		// 0x2067B8
		// FUNCTION: PLAYER 0x00575d00 FOLDED
		// FUNCTION: STUDIO 0x007aa330 FOLDED
		virtual void onChildRemoved(IndexedTree* child) {}		// 0x57E270
		// STUB: PLAYER 0x00528720
		virtual void onAncestorChanged() {}						// 0x55D6D0
		void setIndexedTreeParent(IndexedTree* newParent);		// 0x206A62
	public:
		IndexedTree();											// 0x2068AA
		virtual ~IndexedTree();									// 0x20691A (complete) / 0x206910 (deleting)
		int numChildren() const						{return children.size();}
		// FUNCTION: PLAYER 0x0082e750
		// FUNCTION: STUDIO 0x009a0d10
		template<class Type>
		Type* getTypedChild(int i) {
			return rbx_static_cast<Type*>(children[i]);
		}
		template<class Type>
		const Type* getConstTypedChild(int i) const {
			return rbx_static_cast<Type*>(children[i]);
		}
		template<class Type>
		Type* getTypedParent() {
			return rbx_static_cast<Type*>(parent);
		}
		template<class Type>
		const Type* getConstTypedParent() const {
			return rbx_static_cast<Type*>(parent);
		}
		template<class Type>
		Type* getRoot() {
			return (parent)
				? parent->getRoot<Type>()
				: rbx_static_cast<Type*>(this);
		}
		template<class Type>
		const Type* getRoot() const {
			return (parent)
				? parent->getRoot<Type>()
				: rbx_static_cast<const Type*>(this);
		}
		// FUNCTION: PLAYER 0x00998780
		// FUNCTION: STUDIO 0x00a58b00
		template<class Type>
		Type* getOneBelowRoot() {
			IndexedTree* above = parent;
			RBXASSERT(above != NULL);							// assert string found at IndexedTree.h:78 in binary
			IndexedTree* answer = this;
			while (above->parent) {
				answer = above;
				above = above->parent;
			}
			return rbx_static_cast<Type*>(answer);
		}

		int getDepth() const {
			return parent ? (parent->getDepth() + 1) : 1;
		}

		template<class Type, class Func>
		inline void visitMeAndChildren(Func func)
		{
			Type* t = rbx_static_cast<Type*>(this);
			func(t);
			for (int i = 0; i < children.size(); ++i) {
				children[i]->visitMeAndChildren<Type, Func>(func);
			}
		}

		template<class Type, class Func>
		inline void visitConstMeAndChildren(Func func) const
		{
			const Type* t = rbx_static_cast<const Type*>(this);
			func(t);
			for (int i = 0; i < children.size(); ++i) {
				children[i]->visitConstMeAndChildren<Type, Func>(func);
			}
		}

		template<class Type, class Func>
		inline void visitDescendents(Func func)
		{
			for (int i = 0; i < children.size(); ++i) {
				children[i]->visitMeAndChildren<Type, Func>(func);
			}
		}

		template<class Type, class Func>
		inline void visitConstDescendents(Func func) const
		{
			for (int i = 0; i < children.size(); ++i) {
				children[i]->visitConstMeAndChildren<Type, Func>(func);
			}
		}

	};

} // namespace
