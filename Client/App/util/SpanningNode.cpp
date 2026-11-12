/* Copyright 2003-2006 ROBLOX Corporation, All Rights Reserved */

// SpanningNode.cpp - October 2013 reconstruction
//
// This module is (almost) header-only. Evidence from the Oct 2013 studio binary:
//
//  mac: 0x24E96A  RBX::SpanningNode::setEdgeToParent(SpanningEdge*)
//            stores the edge at this+0x28 (the single SpanningEdge* member) and
//            returns it. It is a one-line member defined inline in SpanningNode.h
//            territory; defined out-of-line here to match the binary's symbol set.
//
//  mac: 0x592D02  RBX::SpanningNode::~SpanningNode()  (D1, complete)
//  mac: 0x592D0C  RBX::SpanningNode::~SpanningNode()  (D0, deleting)
//            Both only call RBX::IndexedMesh::~IndexedMesh(this). The 2016 source
//            declares an empty inline ~SpanningNode(); the binary emits real
//            destructor symbols because IndexedMesh's dtor is out-of-line.
//
//  mac: 0x24F6BC  RBX::SpanningNode::getDepth(SpanningNode*) - recursive parent walk,
//            matches the static inline in SpanningNode.h exactly.

#include "Util/SpanningNode.h"

namespace RBX {

// mac: 0x24E96A
void SpanningNode::setEdgeToParent(SpanningEdge* edge)
{
	edgeToParent = edge;
}

// mac: 0x592D02 / 0x592D0C - calls IndexedMesh::~IndexedMesh only
// STUB: PLAYER 0x00753900
// STUB: STUDIO 0x00892ca0
SpanningNode::~SpanningNode()
{
}

} // namespace
