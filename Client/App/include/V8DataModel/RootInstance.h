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

	extern const char* const sRootInstance;

	class RootInstance
		: public DescribedNonCreatable<RootInstance, ModelInstance, sRootInstance>
	{
	public:
		RootInstance() {}
	};

} // namespace RBX
