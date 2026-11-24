/* V8DataModel/RootInstance.h — October 2013 (workspace-root intermediate shell).
   Evidence: Described<RootInstance, sRootInstance, ModelInstance> 0x46D570
     (inner only — no Product wrapper, same shape as FaceInstance);
     NonFactoryProduct<Workspace, RootInstance, sWorkspace> 0x519ADE/0x2904A0.
     sRootInstance = "RootInstance" (0.125 metadata class list; literal
     TODO-verify vs binary).
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/RootInstance.h */

#pragma once

#include "V8DataModel/ModelInstance.h"

namespace RBX {

	class PartInstance;

	extern const char* const sRootInstance;

	class RootInstance
		: public DescribedNonCreatable<RootInstance, ModelInstance, sRootInstance>
	{
	public:
		RootInstance() {}

		// PLAYER 0x007e33f0 (iOS __ZN3RBX12RootInstance15publicInsertRaw...EPS4_RS1_...bb): two
		// trailing bools in 2013 (InsertService::insert 0x007d6a20 passes true, true).
		void publicInsertRaw(const Instances& instances, Instance* requestedParent,
			std::vector<weak_ptr<PartInstance> >& parts, bool joinParts, bool suppressMove);
	};

} // namespace RBX
