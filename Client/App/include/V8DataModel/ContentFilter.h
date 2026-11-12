/* V8DataModel/ContentFilter.h — October 2013 STUB (minimal, unblocks DataModel).
   Evidence: ServiceProvider::create<ContentFilter> from DataModel::initializeContents
     0x2D3336; "ContentFilter" string in both 2013 PEs. No March 2016 counterpart
     (removed/renamed by 2016) — bases are inferred (Service, since it parents to the
     DataModel; NonCreatable like other providers).
   TODO: full decompile (filter-string machinery).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/ContentFilter.h */

#pragma once

#include "V8Tree/Instance.h"
#include "V8Tree/Service.h"

namespace RBX {

	extern const char* const sContentFilter;

	class ContentFilter
		: public DescribedNonCreatable<ContentFilter, Instance, sContentFilter, Reflection::ClassDescriptor::RUNTIME_LOCAL>
		, public Service
	{
	public:
		ContentFilter() {}
	};

} // namespace RBX
