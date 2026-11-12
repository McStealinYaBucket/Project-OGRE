/* V8World/KernelJoint.h -- October 2013 (kernel-joint type identity).
   Minimal subclass: GroundStage::onKernelJointAdded/Removing take
   KernelJoint* (batches 208/219a) and touch only the Edge interface
   (prims + downstreamOfStage), so no members/methods are modeled.
   getJointType override returns KERNEL_JOINT (binary dispatches type 12
   here -- GroundStage::onEdgeAdded gates edgeType==0 && jointType==12).
   C2 lives elsewhere (no in-tree construction sites); none declared.
   Original file: Client/App/v8world/KernelJoint.h (path inferred) */

#pragma once

#include "V8World/Joint.h"

namespace RBX {

	class KernelJoint : public Joint
	{
	public:
		virtual int getJointType() const { return KERNEL_JOINT; }
	};

} // namespace RBX
