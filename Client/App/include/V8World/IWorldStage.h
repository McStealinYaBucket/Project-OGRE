/* V8World/IWorldStage.h -- October 2013 (physics stage interface).
   2013 MSVC vftable (AssemblyStage 0x00b94b28, JointStage 0x00bb2b00): +0x00 dtor,
     +0x04 getStageType, +0x08 IStage::getKernel, +0x0c onEdgeAdded, +0x10
     onEdgeRemoving, +0x14 getMetric (base body PLAYER 0x008e4760; iOS names).
   DELTA: onEdgeAdded/onEdgeRemoving are non-pure in the binary (base bodies in
     IWorldStage.cpp) and are declared pure here so a concrete stage is required.
   Original file: Client/App/include/V8World/IWorldStage.h (2013 assert path;
     the getMetric assert below must stay on line 44, as in 2013). */


#pragma once

#include "V8Kernel/IStage.h"

namespace RBX {

	class Edge;
	class Kernel;

	class IWorldStage
		: public IStage
	{
	public:
		// MetricType: 2016 shape (2013 RTTI names IWorldStage::MetricType).
		typedef enum { NUM_CONTACTSTAGE_CONTACTS, NUM_STEPPING_CONTACTS,
			NUM_TOUCHING_CONTACTS, MAX_TREE_DEPTH } MetricType;

		virtual ~IWorldStage() {}

		// Downstream accessor (IWorldStage.cpp :11/:18 asserts). The +8 child
		// is IWorldStage* for every stage except Simulate (terminal Kernel*).
		IWorldStage* getDownstreamWS() const {
			return static_cast<IWorldStage*>(downstream);
		}

		// +0x08 is IStage::getKernel (no stage overrides it).
		// +0x0c / +0x10
		virtual void onEdgeAdded(Edge* edge) = 0;
		virtual void onEdgeRemoving(Edge* edge) = 0;
		// +0x14 (AssemblyStage vftable 0x00b94b28 slot 5): the assert is
		// IWorldStage.h line 44 in 2013 (string at PLAYER 0x00b90b78).
		// STUB: PLAYER 0x008e4760
		virtual int getMetric(MetricType metricType) {
			RBXASSERT(getDownstreamWS());
			return getDownstreamWS()->getMetric(metricType);
		}
	};

} // namespace RBX
