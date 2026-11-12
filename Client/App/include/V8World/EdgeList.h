/* V8World/EdgeList.h -- October 2013 (parallel-array edge adjacency).
   Evidence: NO node struct (2016 merged to vector<Entry>; 2013 keeps two
     parallel G3D::Array<T,10,32>): owner +0, edges +4{data,num}, others +20,
     36 B total; ctor 0x592CB4 / dtor 0x592BAC (both-sizes==0 asserts) /
     getEdge 0x5924CC / getOther 0x56BC50 / getNext 0x590190 /
     insertEdge 0x59027E / removeEdge 0x590356 (swap-hole + index fixups).
     Primitive embeds two: contacts +128, joints +164.
   All methods header-inline (CRT rule, session 72: Array heap stays same-side;
   matches the binary's inline-helper shape for the tiny methods).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8world/EdgeList.h (path inferred) */

#pragma once

#include "G3D/Array.h"
#include "V8World/Edge.h"
#include "rbx/Debug.h"

namespace RBX {

	class Primitive;

	class EdgeList
	{
	public:
		explicit EdgeList(Primitive* owner)
			: owner(owner)
		{}
		// STUB: PLAYER 0x00755400
		// STUB: STUDIO 0x00894720
		~EdgeList()
		{
			RBXASSERT(edges.size() == 0 && others.size() == 0);
		}

		// mac: 0x5924CC (+ cross-connection assert).
		// STUB: PLAYER 0x00754560
		// STUB: STUDIO 0x00893910
		Edge* getEdge(int i) const
		{
			RBXASSERT(i >= 0 && i < edges.size());
			RBXASSERT(edges[i]->otherPrimitive(owner) == others[i]);
			return edges[i];
		}
		// mac: 0x56BC50.
		// STUB: PLAYER 0x0071b6f0
		// STUB: STUDIO 0x007b7170
		Primitive* getOther(int i) const
		{
			RBXASSERT(i >= 0 && i < edges.size());
			RBXASSERT(edges[i]->otherPrimitive(owner) == others[i]);
			return others[i];
		}
		// mac: 0x590190.
		// STUB: PLAYER 0x007546b0
		// STUB: STUDIO 0x008939e0
		Edge* getNext(Edge* e, Primitive* p) const
		{
			RBXASSERT(e != NULL);
			RBXASSERT(e->links(p));
			int idx = e->getIndex(p) + 1;
			return idx < edges.size() ? edges[idx] : NULL;
		}
		// mac: 0x59027E (Primitive.cpp:236 "e->getIndex(owner) == -1").
		// STUB: PLAYER 0x007563f0
		// STUB: STUDIO 0x00895710
		void insertEdge(Edge* e)
		{
			RBXASSERT(e->getIndex(owner) == -1);  // Primitive.cpp:236
			e->setIndex(owner, edges.size());
			edges.append(e);
			others.append(e->otherPrimitive(owner));
		}
		// mac: 0x590356 (Primitive.cpp:250 "removeIndex >= 0" / :251
		// "edges[removeIndex] == e"; unconditional swap-hole on both arrays).
		// STUB: PLAYER 0x00756040
		// STUB: STUDIO 0x00895360
		void removeEdge(Edge* e)
		{
			int idx = e->getIndex(owner);
			RBXASSERT(idx >= 0);  // Primitive.cpp:250 "removeIndex >= 0"
			RBXASSERT(edges[idx] == e);  // Primitive.cpp:251 "edges[removeIndex] == e"
			int last = edges.size() - 1;
			edges[idx] = edges[last];
			others[idx] = others[last];
			edges[idx]->setIndex(owner, idx);
			edges.resize(last);
			others.resize(last);
			e->setIndex(owner, -1);
		}
		// Bring-up size query (binary reads sizes directly; not a method there).
		int size() const { return edges.size(); }

	private:
		Primitive* owner;                     // +0
		G3D::Array<Edge*,10,32> edges;        // +4
		G3D::Array<Primitive*,10,32> others;  // +20
	};

} // namespace RBX
