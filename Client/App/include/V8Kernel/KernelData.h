/* V8Kernel/KernelData.h -- October 2013 (kernel body/connector lists, 0xB0).
   Evidence: C2 0x530AE8 (334B: 11 G3D::Array inits at +0/+16/+32/+48/+64/
     +80/+96/+112/+128/+144/+160) / D2 0x530472 (1267B: 11 empty asserts,
     KernelData.h:38-48, names banked in order below) (batch221b).
   Members (binary order + G3D::Array element types from C2 template
   instantiations; assert names from D2 strings):
     +0   Array<SimBody*>   freeFallBodies    (:38)
     +16  Array<SimBody*>   realTimeBodies    (:39)
     +32  Array<SimBody*>   jointBodies       (:40)
     +48  Array<SimBody*>   contactBodies     (:41)
     +64  Array<Body*>      leafBodies        (:42)
     +80  Array<Point*>     points            (:43)
     +96  Array<Connector*> humanoidConnectors      (:44)
     +112 Array<Connector*> secondPassConnectors    (:45)
     +128 Array<Connector*> realTimeConnectors      (:46)
     +144 Array<Connector*> jointConnectors         (:47)
     +160 Array<Connector*> contactConnectors       (:48)
   DELTA vs March 2016 KernelData.h (first-party diff aid): 2016 uses
   IndexArray x12 (adds buoyancyConnectors); 2013 uses G3D::Array x11
   (no buoyancy -- absent from C2/D2). Element classes forward-declared
   (Body module absent); arrays of pointers need no complete type.
   Original file: Client/App/include/V8Kernel/KernelData.h (path from
   assert strings) */

#pragma once

#include "G3D/Array.h"

namespace RBX {

	class SimBody;
	class Body;
	class Point;
	class Connector;

	class KernelData
	{
	public:
		G3D::Array<SimBody*> freeFallBodies;      // +0
		G3D::Array<SimBody*> realTimeBodies;      // +16
		G3D::Array<SimBody*> jointBodies;         // +32
		G3D::Array<SimBody*> contactBodies;       // +48
		G3D::Array<Body*> leafBodies;             // +64
		G3D::Array<Point*> points;                // +80
		G3D::Array<Connector*> humanoidConnectors;    // +96
		G3D::Array<Connector*> secondPassConnectors;  // +112
		G3D::Array<Connector*> realTimeConnectors;    // +128
		G3D::Array<Connector*> jointConnectors;       // +144
		G3D::Array<Connector*> contactConnectors;     // +160

		KernelData();
		~KernelData();

		// List management (batch221b/254: index asserts + back-swap
		// fastRemove spellings; validate via SimBody predicates).
		void insertBody(Body* body);
		void removeBody(Body* body);
		void addBodyToNewList(SimBody* simBody);
		void removeBodyFromCurrentList(SimBody* simBody);
		void addLeafBody(Body* body);
		void addLeafBodies(Body* body);
		void removeLeafBodies(Body* body);
		// (batch221b: :88 single-list + kind routing below; returns kind).
		int addConnector(Connector* connector);
		// (batch221b: all-clear early-out + per-list fastRemove + sim
		// decrements + symmetry pair on contact + leaf arm).
		void removeConnector(Connector* connector);
	};

} // namespace RBX
