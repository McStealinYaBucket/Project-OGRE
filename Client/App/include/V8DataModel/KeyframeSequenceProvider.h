/* V8DataModel/KeyframeSequenceProvider.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<KeyframeSequenceProvider> from DataModel::
     initializeContents 0x2D3336; "KeyframeSequenceProvider" string in both 2013 PEs.
   Bases (DescribedNonCreatable + Service) carried from March 2016 header.
   TODO: full decompile (keyframe cache machinery).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/KeyframeSequenceProvider.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sKeyframeSequenceProvider;

	class KeyframeSequenceProvider
		: public DescribedNonCreatable<KeyframeSequenceProvider, Instance, sKeyframeSequenceProvider, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		KeyframeSequenceProvider() {}
	};

} // namespace RBX
